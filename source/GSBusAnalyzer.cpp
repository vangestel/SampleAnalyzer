#include "GSBusAnalyzer.h"
#include "GSBusAnalyzerSettings.h"
#include <AnalyzerChannelData.h>
#include <AnalyzerHelpers.h>

#include <sstream>
#include <string>
#include <cstring>

GSBusAnalyzer::GSBusAnalyzer()
:	Analyzer2(),  
	mSettings( new GSBusAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

GSBusAnalyzer::~GSBusAnalyzer()
{
	KillThread();
}

void GSBusAnalyzer::SetupResults()
{
	mResults.reset( new GSBusAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mCommandChannel );
	mResults->AddChannelBubblesWillAppearOn(mSettings->mStatusChannel);
}

void GSBusAnalyzer::WorkerThread()
{
	//UpArrow, DownArrow
	if (mSettings->mDataValidEdge == AnalyzerEnums::NegEdge)
		mArrowMarker = AnalyzerResults::DownArrow;
	else
		mArrowMarker = AnalyzerResults::UpArrow;

	mClock = GetAnalyzerChannelData(mSettings->mClockChannel);
	mFrame = GetAnalyzerChannelData(mSettings->mFrameChannel);
	mCommand = GetAnalyzerChannelData(mSettings->mCommandChannel);
	mStatus = GetAnalyzerChannelData(mSettings->mStatusChannel);

	SetupForGettingFirstBit();
	SetupForGettingFirstFrame();

	for (; ; )
	{
		GetFrame();
		AnalyzeFrame();

		mResults->CommitResults();
		ReportProgress(mClock->GetSampleNumber());
		CheckIfThreadShouldExit();
	}
}

void GSBusAnalyzer::SetupForGettingFirstBit()
{
	if (mSettings->mDataValidEdge == AnalyzerEnums::PosEdge)
	{
		//we want to start out low, so the next time we advance, it'll be a rising edge.
		if (mClock->GetBitState() == BIT_HIGH)
			mClock->AdvanceToNextEdge(); //now we're low.
	}
	else
	{
		//we want to start out low, so the next time we advance, it'll be a falling edge.
		if (mClock->GetBitState() == BIT_LOW)
			mClock->AdvanceToNextEdge(); //now we're high.
	}
}

void GSBusAnalyzer::SetupForGettingFirstFrame()
{
	GetNextBit(mLastCommand, mLastStatus, mLastFrame, mLastSample); //we have to throw away one bit to get enough history on the FRAME line.

	for (; ; )
	{
		GetNextBit(mCurrentCommand, mCurrentStatus, mCurrentFrame, mCurrentSample);

		// The edge at which the frame sync signal transitions from high to low is the first valid frame bit.
		if (mCurrentFrame == BIT_LOW && mLastFrame == BIT_HIGH)
		{
			return;
		}

		mLastFrame = mCurrentFrame;
		mLastCommand = mCurrentCommand;
		mLastStatus = mCurrentStatus;
		mLastSample = mCurrentSample;
	}
}

void GSBusAnalyzer::GetFrame()
{
	//on entering this function: 
	//mCurrentFrame and mCurrentData are the values of the first bit -- that belongs to us -- in the frame.
	//mLastFrame and mLastData are the values from the bit just before.

	// Clear the valid bits buffers and the valid edges index buffers.
	mCommandBits.clear();
	mStatusBits.clear();
	mCommandValidEdges.clear();
	mStatusValidEdges.clear();

	// The current data bit is already valid, so add it to the valid bits buffer.
	mCommandBits.push_back(mCurrentCommand);
	mStatusBits.push_back(mCurrentStatus);
	mCommandValidEdges.push_back(mCurrentSample);
	mStatusValidEdges.push_back(mCurrentSample);

	mLastFrame = mCurrentFrame;
	mLastCommand = mCurrentCommand;
	mLastStatus = mCurrentStatus;
	mLastSample = mCurrentSample;

	for (; ; )
	{
		GetNextBit(mCurrentCommand, mCurrentStatus, mCurrentFrame, mCurrentSample);

		// The edge at which the frame sync signal transitions from low to high is the last valid frame bit.
		if (mCurrentFrame == BIT_LOW && mLastFrame == BIT_HIGH)
		{
			return;
		}

		// Include the last valid bit.
		mCommandBits.push_back(mCurrentCommand);
		mStatusBits.push_back(mCurrentStatus);
		mCommandValidEdges.push_back(mCurrentSample);
		mStatusValidEdges.push_back(mCurrentSample);

		mLastFrame = mCurrentFrame;
		mLastCommand = mCurrentCommand;
		mLastStatus = mCurrentStatus;
		mLastSample = mCurrentSample;
	}
}

void GSBusAnalyzer::AnalyzeFrame()
{
	U32 num_bits = mCommandBits.size();
	U32 num_channels = mSettings->mChannelsPerFrame;

	if ((num_bits % num_channels) != 0)
	{
		Frame frame;
		frame.mType = 255;
		frame.mFlags = DISPLAY_AS_ERROR_FLAG;
		frame.mStartingSampleInclusive = mCommandValidEdges.front();
		frame.mEndingSampleInclusive = mCommandValidEdges.back();
		mResults->AddFrame(frame);
		return;
	}

	U32 bits_per_channel = num_bits / num_channels;
	U32 databits_per_channel = mSettings->mDataBitsPerChannel;

	if (bits_per_channel < databits_per_channel)
	{
		Frame frame;
		frame.mType = 254;
		frame.mFlags = DISPLAY_AS_ERROR_FLAG;
		frame.mStartingSampleInclusive = mCommandValidEdges.front();
		frame.mEndingSampleInclusive = mCommandValidEdges.back();
		mResults->AddFrame(frame);
		return;
	}

	for (U8 i = 0; i < mSettings->mChannelsPerFrame; i++)
	{
		AnalyzeSubFrame(i * bits_per_channel, databits_per_channel, i);
	}
}

void GSBusAnalyzer::AnalyzeSubFrame(U32 starting_index, U32 num_bits, U8 channel_index)
{
	// Convert the data bit buffer for each channel/subframe to its numeric value.
	U64 commandResult = 0;
	U64 statusResult = 0;
	U32 target_count = starting_index + num_bits;

	if (mSettings->mShiftOrder == AnalyzerEnums::LsbFirst)
	{
		U64 bit_value = 1ULL;
		for (U32 i = starting_index; i < target_count; i++)
		{
			if (mCommandBits[i] == BIT_HIGH)
				commandResult |= bit_value;

			bit_value <<= 1;
		}

		bit_value = 1ULL;
		for (U32 i = starting_index; i < target_count; i++)
		{
			if (mStatusBits[i] == BIT_HIGH)
				statusResult |= bit_value;

			bit_value <<= 1;
		}
	}
	else
	{
		U64 bit_value = 1ULL << (num_bits - 1);
		for (U32 i = starting_index; i < target_count; i++)
		{
			if (mCommandBits[i] == BIT_HIGH)
				commandResult |= bit_value;

			bit_value >>= 1;
		}

		bit_value = 1ULL << (num_bits - 1);
		for (U32 i = starting_index; i < target_count; i++)
		{
			if (mStatusBits[i] == BIT_HIGH)
				statusResult |= bit_value;

			bit_value >>= 1;
		}
	}

	// Assign the numeric result data to the frame object.
	Frame frame;
	frame.mData1 = commandResult;
	frame.mData2 = statusResult;

	// Set the channel index as the frame type.
	frame.mType = channel_index;

	// Set other frame data.
	frame.mFlags = 0;
	frame.mStartingSampleInclusive = mCommandValidEdges[starting_index];
	frame.mEndingSampleInclusive = mCommandValidEdges[starting_index + num_bits - 1];

	// Add the frame to the aggregated results.
	mResults->AddFrame(frame);
}

void GSBusAnalyzer::GetNextBit(BitState& command, BitState& status, BitState& frame, U64& sample_number)
{
	// Advance to the next edge, which will be negative.
	mClock->AdvanceToNextEdge();
	U64 data_valid_sample = mClock->GetSampleNumber();

	mCommand->AdvanceToAbsPosition(data_valid_sample);
	command = mCommand->GetBitState();

	mStatus->AdvanceToAbsPosition(data_valid_sample);
	status = mStatus->GetBitState();

	mFrame->AdvanceToAbsPosition(data_valid_sample);
	frame = mFrame->GetBitState();

	sample_number = data_valid_sample;

	mResults->AddMarker(data_valid_sample, mArrowMarker, mSettings->mClockChannel);

	// Advance to the next positive edge, so that the next one is again a negative edge.
	mClock->AdvanceToNextEdge();
}

U32 GSBusAnalyzer::GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels)
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData(newest_sample_requested, sample_rate, simulation_channels);
}

U32 GSBusAnalyzer::GetMinimumSampleRateHz()
{
	return 100000000;
}

bool GSBusAnalyzer::NeedsRerun()
{
	return false;
}

const char* GSBusAnalyzer::GetAnalyzerName() const
{
	return "GSBus";
}

const char* GetAnalyzerName()
{
	return "GSBus";
}

Analyzer* CreateAnalyzer()
{
	return new GSBusAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}