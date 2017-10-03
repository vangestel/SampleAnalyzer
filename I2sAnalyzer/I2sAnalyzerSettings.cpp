#include "I2sAnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>
#include <cstring>
#include <stdio.h>

#pragma warning(disable: 4996) //warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

I2sAnalyzerSettings::I2sAnalyzerSettings()
:	mClockChannel( UNDEFINED_CHANNEL ),
	mFrameChannel( UNDEFINED_CHANNEL ),
	mDataChannel( UNDEFINED_CHANNEL ),

	mShiftOrder( AnalyzerEnums::MsbFirst ),
	mDataValidEdge( AnalyzerEnums::NegEdge ),
	mBitsPerWord( 16 ),

	mWordAlignment( LEFT_ALIGNED ),
	mFrameType( FRAME_TRANSITION_ONCE_EVERY_WORD ),
	mBitAlignment( BITS_SHIFTED_RIGHT_1 ),
	mSigned( AnalyzerEnums::UnsignedInteger ),
	mWordSelectInverted( WS_NOT_INVERTED )
{
	mClockChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mClockChannelInterface->SetTitleAndTooltip( "CLOCK", "Clock, aka I2S SCK - Continuous Serial Clock, aka Bit Clock" );
	mClockChannelInterface->SetChannel( mClockChannel );

	mFrameChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mFrameChannelInterface->SetTitleAndTooltip( "FRAME", "Frame Delimiter / aka I2S WS - Word Select, aka Sampling Clock" );
	mFrameChannelInterface->SetChannel( mFrameChannel );

	mDataChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mDataChannelInterface->SetTitleAndTooltip( "DATA", "Data, aka I2S SD - Serial Data" );
	mDataChannelInterface->SetChannel( mDataChannel );




	mShiftOrderInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mShiftOrderInterface->SetTitleAndTooltip( "", "Specify if data comes in MSB first, or LSB first." );
	mShiftOrderInterface->AddNumber( AnalyzerEnums::MsbFirst, "DATA arrives MSB first", "Data arrives most significant bit (MSB) first" );
	mShiftOrderInterface->AddNumber( AnalyzerEnums::LsbFirst, "DATA arrives LSB first", "Data arrives least significant bit (LSB) first" );
	mShiftOrderInterface->SetNumber( mShiftOrder );

	mDataValidEdgeInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mDataValidEdgeInterface->SetTitleAndTooltip( "", "Specify if data is valid (should be read) on the rising, or falling clock edge." );
	mDataValidEdgeInterface->AddNumber( AnalyzerEnums::NegEdge, "DATA is valid (should be read) on the CLOCK falling edge", "" );
	mDataValidEdgeInterface->AddNumber( AnalyzerEnums::PosEdge, "DATA is valid (should be read) on the CLOCK rising edge", "" );
	mDataValidEdgeInterface->SetNumber( mDataValidEdge );

	mBitsPerWordInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mBitsPerWordInterface->SetTitleAndTooltip( "", "Specify the number of audio bits/word.  Any additional bits will be ignored" );
	char str[256];
	for( U32 i=2; i <= 64; i++ )
	{
		sprintf( str, "%d Bits/Word (Audio bit depth, bits/sample)", i );
		mBitsPerWordInterface->AddNumber( i, str, "Specify the number of audio bits/word.  Any additional bits will be ignored" );
	}
	mBitsPerWordInterface->SetNumber( mBitsPerWord );



	//enum PcmFrameType { FRAME_TRANSITION_TWICE_EVERY_WORD, FRAME_TRANSITION_ONCE_EVERY_WORD, FRAME_TRANSITION_TWICE_EVERY_FOUR_WORDS };
	mFrameTypeInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mFrameTypeInterface->SetTitleAndTooltip( "", "Specify the type of frame signal used." );
	mFrameTypeInterface->AddNumber( FRAME_TRANSITION_TWICE_EVERY_WORD, "FRAME signal transitions (changes state) twice each word.", "" );
	mFrameTypeInterface->AddNumber( FRAME_TRANSITION_ONCE_EVERY_WORD, "FRAME signal transitions (changes state) once each word. (I2S, PCM standard)", "" );
	mFrameTypeInterface->AddNumber( FRAME_TRANSITION_TWICE_EVERY_FOUR_WORDS, "FRAME signal transitions(changes state) twice every four (4) words.", "" );
	mFrameTypeInterface->SetNumber( mFrameType );

	mWordAlignmentInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mWordAlignmentInterface->SetTitleAndTooltip( "", "Specify whether data bits are left or right aligned wrt FRAME edges. Only needed if more bits are sent than needed each frame, and additional bits are ignored." );
	mWordAlignmentInterface->AddNumber( LEFT_ALIGNED, "DATA bits are left-aligned with respect to FRAME edges", "Specify whether data bits are left or right aligned wrt FRAME edges. Only needed if more bits are sent than needed each frame, and additional bits are ignored." );
	mWordAlignmentInterface->AddNumber( RIGHT_ALIGNED, "DATA bits are right-aligned with respect to FRAME edges", "Specify whether data bits are left or right aligned wrt FRAME edges. Only needed if more bits are sent than needed each frame, and additional bits are ignored." );
	mWordAlignmentInterface->SetNumber( mWordAlignment );

	//enum PcmBitAlignment { FIRST_FRAME_BIT_BELONGS_TO_PREVIOUS_WORD, FIRST_FRAME_BIT_BELONGS_TO_CURRENT_WORD };
	mBitAlignmentInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mBitAlignmentInterface->SetTitleAndTooltip( "", "Specify the bit alignment type to use." );
	mBitAlignmentInterface->AddNumber( BITS_SHIFTED_RIGHT_1, "Bits are right-shifted by one with respect to FRAME edges (I2S typical)", "In I2S, bits are typically right shifted by one" );
	mBitAlignmentInterface->AddNumber( NO_SHIFT, "Bits are not shifted with respect to FRAME edges (PCM typical)", "In PCM, bits are typically not shifted" );
	mBitAlignmentInterface->SetNumber( mBitAlignment );

	mSignedInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mSignedInterface->SetTitleAndTooltip( "", "Select whether samples are unsigned or signed values (only shows up if the display type is decimal)" );
	mSignedInterface->AddNumber( AnalyzerEnums::UnsignedInteger, "Samples are unsigned numbers", "Interpret samples as unsigned integers" );
	mSignedInterface->AddNumber( AnalyzerEnums::SignedInteger, "Samples are signed (two's complement)", "Interpret samples as signed integers -- only when display type is set to decimal" );
	mSignedInterface->SetNumber( mSigned );


	mWordSelectInvertedInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mWordSelectInvertedInterface->SetTitleAndTooltip("", "Select weather WS high is channel 1 or channel 2");
	mWordSelectInvertedInterface->AddNumber( WS_NOT_INVERTED, "Word select high is channel 2 (right) (I2S typical)", "when word select (FRAME) is logic 1, data is channel 2.");
	mWordSelectInvertedInterface->AddNumber( WS_INVERTED, "Word select high is channel 1 (left) (inverted)", "when word select (FRAME) is logic 1, data is channel 1.");
	mWordSelectInvertedInterface->SetNumber( mWordSelectInverted );

	AddInterface( mClockChannelInterface.get() );
	AddInterface( mFrameChannelInterface.get() );
	AddInterface( mDataChannelInterface.get() );
	AddInterface( mShiftOrderInterface.get() );
	AddInterface( mDataValidEdgeInterface.get() );
	AddInterface( mBitsPerWordInterface.get() );
	AddInterface( mFrameTypeInterface.get() );
	AddInterface( mWordAlignmentInterface.get() );
	AddInterface( mBitAlignmentInterface.get() );
	AddInterface( mSignedInterface.get() );
	AddInterface( mWordSelectInvertedInterface.get() );

	//AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mClockChannel, "PCM CLOCK", false );
	AddChannel( mFrameChannel, "PCM FRAME", false );
	AddChannel( mDataChannel, "PCM DATA", false );
}

I2sAnalyzerSettings::~I2sAnalyzerSettings()
{
}

void I2sAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mClockChannelInterface->SetChannel( mClockChannel );
	mFrameChannelInterface->SetChannel( mFrameChannel );
	mDataChannelInterface->SetChannel( mDataChannel );

	mShiftOrderInterface->SetNumber( mShiftOrder );
	mDataValidEdgeInterface->SetNumber( mDataValidEdge );
	mBitsPerWordInterface->SetNumber( mBitsPerWord );

	mWordAlignmentInterface->SetNumber( mWordAlignment );
	mFrameTypeInterface->SetNumber( mFrameType );
	mBitAlignmentInterface->SetNumber( mBitAlignment );

	mSignedInterface->SetNumber( mSigned );

	mWordSelectInvertedInterface->SetNumber( mWordSelectInverted );
}

bool I2sAnalyzerSettings::SetSettingsFromInterfaces()
{
	Channel clock_channel = mClockChannelInterface->GetChannel();
	if( clock_channel == UNDEFINED_CHANNEL )
	{
		SetErrorText( "Please select a channel for I2S/PCM CLOCK signal" );
		return false;
	}

	Channel frame_channel = mFrameChannelInterface->GetChannel();
	if( frame_channel == UNDEFINED_CHANNEL )
	{
		SetErrorText( "Please select a channel for I2S/PCM FRAME signal" );
		return false;
	}

	Channel data_channel = mDataChannelInterface->GetChannel();
	if( data_channel == UNDEFINED_CHANNEL )
	{
		SetErrorText( "Please select a channel for I2S/PCM DATA signal" );
		return false;
	}

	if( ( clock_channel == frame_channel ) || ( clock_channel == data_channel ) || ( frame_channel == data_channel ) )
	{
		SetErrorText( "Please select different channels for the I2S/PCM signals" );
		return false;
	}

	mClockChannel = clock_channel;
	mFrameChannel = frame_channel;
	mDataChannel = data_channel;

	mShiftOrder = AnalyzerEnums::ShiftOrder( U32( mShiftOrderInterface->GetNumber() ) );
	mDataValidEdge = AnalyzerEnums::EdgeDirection( U32( mDataValidEdgeInterface->GetNumber() ) );
	mBitsPerWord = U32( mBitsPerWordInterface->GetNumber() );

	mWordAlignment = PcmWordAlignment( U32( mWordAlignmentInterface->GetNumber() ) );
	mFrameType = PcmFrameType( U32( mFrameTypeInterface->GetNumber() ) );
	mBitAlignment = PcmBitAlignment( U32( mBitAlignmentInterface->GetNumber() ) );

	mSigned = AnalyzerEnums::Sign( U32( mSignedInterface->GetNumber() ) );

	mWordSelectInverted = PcmWordSelectInverted( U32( mWordSelectInvertedInterface->GetNumber() ) );

	//AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );

	ClearChannels();
	AddChannel( mClockChannel, "PCM CLOCK", true );
	AddChannel( mFrameChannel, "PCM FRAME", true );
	AddChannel( mDataChannel, "PCM DATA", true );

	return true;
}

void I2sAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	const char* name_string;	//the first thing in the archive is the name of the protocol analyzer that the data belongs to.
	text_archive >> &name_string;
	if( strcmp( name_string, "SaleaeI2sPcmAnalyzer" ) != 0 )
		AnalyzerHelpers::Assert( "SaleaeI2sPcmAnalyzer: Provided with a settings string that doesn't belong to us;" );

	text_archive >>  mClockChannel;
	text_archive >>  mFrameChannel;
	text_archive >>  mDataChannel;

	text_archive >> *(U32*)&mShiftOrder;
	text_archive >> *(U32*)&mDataValidEdge;
	text_archive >> mBitsPerWord;

	text_archive >> *(U32*)&mWordAlignment;
	text_archive >> *(U32*)&mFrameType;
	text_archive >> *(U32*)&mBitAlignment;

	//check to make sure loading it actual works befor assigning the result -- do this when adding settings to an anylzer which has been previously released.
	AnalyzerEnums::Sign sign;
	if( text_archive >> *(U32*)&sign )
		mSigned = sign;

	PcmWordSelectInverted word_inverted;
	if( text_archive >> *(U32*)&word_inverted )
		mWordSelectInverted = word_inverted;

	ClearChannels();
	AddChannel( mClockChannel, "PCM CLOCK", true );
	AddChannel( mFrameChannel, "PCM FRAME", true );
	AddChannel( mDataChannel, "PCM DATA", true );

	UpdateInterfacesFromSettings();
}

const char* I2sAnalyzerSettings::SaveSettings()
{//SaleaeI2sPcmAnalyzer
	SimpleArchive text_archive;

	text_archive << "SaleaeI2sPcmAnalyzer";
	
	text_archive << mClockChannel;
	text_archive << mFrameChannel;
	text_archive << mDataChannel;

	text_archive << mShiftOrder;
	text_archive << mDataValidEdge;
	text_archive << mBitsPerWord;

	text_archive << mWordAlignment;
	text_archive << mFrameType;
	text_archive << mBitAlignment;

	text_archive << mSigned;

	text_archive << mWordSelectInverted;

	return SetReturnString( text_archive.GetString() );
}	
