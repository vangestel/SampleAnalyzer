#include "I2cAnalyzer.h"
#include "I2cAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include "LogicDebug.h"
I2cAnalyzer::I2cAnalyzer()
:	Analyzer2(),  
	mSettings( new I2cAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
	LogicDebug::StartPrintService();  //required if you want debug print statements from the analyzer.
}

I2cAnalyzer::~I2cAnalyzer()
{
	KillThread();
}

void I2cAnalyzer::SetupResults()
{
	mResults.reset( new I2cAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mSdaChannel );
}

void I2cAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();
	mNeedAddress = true;

	mSda = GetAnalyzerChannelData( mSettings->mSdaChannel );
	mScl = GetAnalyzerChannelData( mSettings->mSclChannel );

	AdvanceToStartBit(); 
	mScl->AdvanceToNextEdge(); //now scl is low.

	for( ; ; )
	{
		GetByte();
		CheckIfThreadShouldExit();
	}
}

void I2cAnalyzer::GetByte()
{
	mArrowLocataions.clear();
	U64 value;
	DataBuilder byte;
	byte.Reset( &value, AnalyzerEnums::MsbFirst, 8 );
	U64 starting_stample = 0;
	
	for( U32 i=0; i<8; i++ )
	{
		BitState bit_state;
		U64 scl_rising_edge;
		bool result = GetBit( bit_state, scl_rising_edge );
		if( result == true )
		{
			mArrowLocataions.push_back( scl_rising_edge );
			byte.AddBit( bit_state );

			if( i == 0 )
				starting_stample = scl_rising_edge;
		}else
		{
			return;
		}
	}

	BitState ack_bit_state;
	U64 scl_rising_edge;
	S64 last_valid_sample = mScl->GetSampleNumber();
	bool result = GetBit( ack_bit_state, scl_rising_edge );

	Frame frame;
	frame.mStartingSampleInclusive = starting_stample;
	frame.mEndingSampleInclusive = result ? mScl->GetSampleNumber() : last_valid_sample;
	frame.mData1 = U8( value );

	if( !result )
		frame.mFlags = I2C_MISSING_FLAG_ACK;
	else if( ack_bit_state == BIT_HIGH )
		frame.mFlags = DISPLAY_AS_WARNING_FLAG;
	else
		frame.mFlags = I2C_FLAG_ACK;

	if( mNeedAddress == true )
	{
		mNeedAddress = false;
		frame.mType = I2cAddress;
	}else
	{
		frame.mType = I2cData;
	}
	mResults->AddFrame( frame );

	U32 count = mArrowLocataions.size();
	for( U32 i=0; i<count; i++ )
		mResults->AddMarker( mArrowLocataions[i], AnalyzerResults::UpArrow, mSettings->mSclChannel );

	mResults->CommitResults();
}

bool I2cAnalyzer::GetBit( BitState& bit_state, U64& sck_rising_edge )
{
	//SCL must be low coming into this function
	mScl->AdvanceToNextEdge(); //posedge
	sck_rising_edge = mScl->GetSampleNumber();
	mSda->AdvanceToAbsPosition( sck_rising_edge );  //data read on SCL posedge

	bit_state = mSda->GetBitState();
	bool result = true;

	//this while loop is only important if you need to be careful and check for things that that might happen at the very end of a data set, and you don't want to get stuck waithing on a channel that never changes.
	while( mScl->DoMoreTransitionsExistInCurrentData() == false )
	{
		// there are no more SCL transtitions, at least yet.
		if( mSda->DoMoreTransitionsExistInCurrentData() == true )
		{
			//there ARE some SDA transtions, let's double check to make sure there's still no SDA activity
			if( mScl->DoMoreTransitionsExistInCurrentData() == true )
				break;

			//ok, for sure we can advance to the next SDA edge without running past any SCL events.
			mSda->AdvanceToNextEdge();
			RecordStartStopBit();
			result = false;
		}
	}

	mScl->AdvanceToNextEdge(); //negedge; we'll leave the clock here
	while( mSda->WouldAdvancingToAbsPositionCauseTransition( mScl->GetSampleNumber() - 1 ) == true )
	{
		//clock is high -- SDA changes indicate start, stop, etc.
		mSda->AdvanceToNextEdge();
		RecordStartStopBit();
		result = false;
	}

	return result;
}

void I2cAnalyzer::RecordStartStopBit()
{
	if( mSda->GetBitState() == BIT_LOW )
	{
		//negedge -> START / restart
		mResults->AddMarker( mSda->GetSampleNumber(), AnalyzerResults::Start, mSettings->mSdaChannel );
	}else
	{
		//posedge -> STOP
		mResults->AddMarker( mSda->GetSampleNumber(), AnalyzerResults::Stop, mSettings->mSdaChannel );
	}
	
	mNeedAddress = true;
	mResults->CommitPacketAndStartNewPacket();
	mResults->CommitResults( );

}

void I2cAnalyzer::AdvanceToStartBit()
{
	for( ; ; )
	{
		mSda->AdvanceToNextEdge();

		if( mSda->GetBitState() == BIT_LOW )
		{
			//SDA negedge
			mScl->AdvanceToAbsPosition( mSda->GetSampleNumber() );
			if( mScl->GetBitState() == BIT_HIGH )
				break;
		}	
	}
	mResults->AddMarker( mSda->GetSampleNumber(), AnalyzerResults::Start, mSettings->mSdaChannel );
}

bool I2cAnalyzer::NeedsRerun()
{
	return false;
}

U32 I2cAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 I2cAnalyzer::GetMinimumSampleRateHz()
{
	return 2000000;
}

const char* I2cAnalyzer::GetAnalyzerName() const
{
	return "I2C";
}

const char* GetAnalyzerName()
{
	return "I2C";
}

Analyzer* CreateAnalyzer()
{
	return new I2cAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
