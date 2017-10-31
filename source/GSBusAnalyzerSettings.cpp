#include "GSBusAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

#include <sstream>
#include <cstring>
#include <stdio.h>

GSBusAnalyzerSettings::GSBusAnalyzerSettings()
  : mClockChannel(UNDEFINED_CHANNEL),
	mFrameChannel(UNDEFINED_CHANNEL),
	mCommandChannel(UNDEFINED_CHANNEL),
	mStatusChannel(UNDEFINED_CHANNEL),

	mBitsPerFrame(256),
	mChannelsPerFrame(8),
	mDataBitsPerChannel(24),
	mStatusBitsPerChannel(7),
	mParityBitsPerChannel(1),

	mShiftOrder(AnalyzerEnums::MsbFirst),
	mDataValidEdge(AnalyzerEnums::NegEdge),
	mSigned(AnalyzerEnums::UnsignedInteger)
{
	// START OF GSBUS SETTINGS

	// Channel setup
	mClockChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mClockChannelInterface->SetTitleAndTooltip("CLOCK", "Clock, aka CMD_CLK");
	mClockChannelInterface->SetChannel(mClockChannel);

	mFrameChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mFrameChannelInterface->SetTitleAndTooltip("FRAME", "Frame Sync pulse / aka CMD_FS");
	mFrameChannelInterface->SetChannel(mFrameChannel);

	mCommandChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mCommandChannelInterface->SetTitleAndTooltip("COMMAND", "Command Data, aka CMD_D");
	mCommandChannelInterface->SetChannel(mCommandChannel);

	mStatusChannelInterface.reset(new AnalyzerSettingInterfaceChannel());
	mStatusChannelInterface->SetTitleAndTooltip("STATUS", "Status Data, aka STAT_D");
	mStatusChannelInterface->SetChannel(mStatusChannel);

	// Bits per frame (2-512, default 256)
	mBitsPerFrameInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mBitsPerFrameInterface->SetTitleAndTooltip("", "Specify the number of bits/frame (GSBus standard: 256).  Any additional bits will be ignored");
	char str[256];
	for (U32 i = 1; i <= 256; i++)
	{
		sprintf(str, "%d Bits/Frame", 2 * i);
		mBitsPerFrameInterface->AddNumber(2 * i, str, "Specify the number of bits/frame (GSBus standard: 256).  Any additional bits will be ignored");
	}
	mBitsPerFrameInterface->SetNumber(mBitsPerFrame);

	// Channels per frame (2-16, default 8)
	mChannelsPerFrameInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mChannelsPerFrameInterface->SetTitleAndTooltip("", "Specify the number of channels per frame (GSBus standard: 8).");
	str[256];
	for (U32 i = 1; i <= 8; i++)
	{
		sprintf(str, "%d Channels/Frame", 2 * i);
		mChannelsPerFrameInterface->AddNumber(2 * i, str, "Specify the number of channels per frame (GSBus standard: 8).");
	}
	mChannelsPerFrameInterface->SetNumber(mChannelsPerFrame);

	// Data bits per channel (2-64, default 24)
	mDataBitsPerChannelInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mDataBitsPerChannelInterface->SetTitleAndTooltip("", "Specify the number of data bits/channel (GSBus standard: 24).");
	str[256];
	for (U32 i = 1; i <= 32; i++)
	{
		sprintf(str, "%d DataBits/Channel", 2 * i);
		mDataBitsPerChannelInterface->AddNumber(2 * i, str, "Specify the number of data bits/channel (GSBus standard: 24).");
	}
	mDataBitsPerChannelInterface->SetNumber(mDataBitsPerChannel);

	// Status bits per channel (0-16, default 7)
	mStatusBitsPerChannelInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mStatusBitsPerChannelInterface->SetTitleAndTooltip("", "Specify the number of status bits/channel (GSBus standard: 7).");
	str[256];
	for (U32 i = 0; i <= 16; i++)
	{
		sprintf(str, "%d StatusBits/Channel", i);
		mStatusBitsPerChannelInterface->AddNumber(i, str, "Specify the number of status bits/channel (GSBus standard: 7).");
	}
	mStatusBitsPerChannelInterface->SetNumber(mStatusBitsPerChannel);

	// Parity bits per channel, autocalculated (default 1)
	mParityBitsPerChannel = (mBitsPerFrame / mChannelsPerFrame) - mDataBitsPerChannel - mStatusBitsPerChannel;

	// END OF GSBUS SETTINGS

	mShiftOrderInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mShiftOrderInterface->SetTitleAndTooltip("", "Specify if data comes in MSB first, or LSB first.");
	mShiftOrderInterface->AddNumber(AnalyzerEnums::MsbFirst, "DATA arrives MSB first", "Data arrives most significant bit (MSB) first");
	mShiftOrderInterface->AddNumber(AnalyzerEnums::LsbFirst, "DATA arrives LSB first", "Data arrives least significant bit (LSB) first");
	mShiftOrderInterface->SetNumber(mShiftOrder);

	mDataValidEdgeInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mDataValidEdgeInterface->SetTitleAndTooltip("", "Specify if data is valid (should be read) on the rising, or falling clock edge.");
	mDataValidEdgeInterface->AddNumber(AnalyzerEnums::NegEdge, "DATA is valid (should be read) on the CLOCK falling edge (GSBus standard)", "");
	mDataValidEdgeInterface->AddNumber(AnalyzerEnums::PosEdge, "DATA is valid (should be read) on the CLOCK rising edge", "");
	mDataValidEdgeInterface->SetNumber(mDataValidEdge);

	mSignedInterface.reset(new AnalyzerSettingInterfaceNumberList());
	mSignedInterface->SetTitleAndTooltip("", "Select whether samples are unsigned or signed values (only shows up if the display type is decimal)");
	mSignedInterface->AddNumber(AnalyzerEnums::UnsignedInteger, "Samples are unsigned numbers", "Interpret samples as unsigned integers");
	mSignedInterface->AddNumber(AnalyzerEnums::SignedInteger, "Samples are signed (two's complement)", "Interpret samples as signed integers -- only when display type is set to decimal");
	mSignedInterface->SetNumber(mSigned);

	AddInterface(mClockChannelInterface.get());
	AddInterface(mFrameChannelInterface.get());
	AddInterface(mCommandChannelInterface.get());
	AddInterface(mStatusChannelInterface.get());
	AddInterface(mBitsPerFrameInterface.get());
	AddInterface(mChannelsPerFrameInterface.get());
	AddInterface(mDataBitsPerChannelInterface.get());
	AddInterface(mStatusBitsPerChannelInterface.get());
	AddInterface(mShiftOrderInterface.get());
	AddInterface(mDataValidEdgeInterface.get());
	AddInterface(mSignedInterface.get());

	//AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
	AddExportOption(0, "Export as text/csv file");
	AddExportExtension(0, "text", "txt");
	AddExportExtension(0, "csv", "csv");

	ClearChannels();
	AddChannel(mClockChannel, "CLOCK", false);
	AddChannel(mFrameChannel, "FRAME", false);
	AddChannel(mCommandChannel, "COMMAND", false);
	AddChannel(mStatusChannel, "STATUS", false);
}

GSBusAnalyzerSettings::~GSBusAnalyzerSettings()
{
}

bool GSBusAnalyzerSettings::SetSettingsFromInterfaces()
{
	Channel clock_channel = mClockChannelInterface->GetChannel();
	if (clock_channel == UNDEFINED_CHANNEL)
	{
		SetErrorText("Please select a channel for CMD_CLK signal");
		return false;
	}

	Channel frame_channel = mFrameChannelInterface->GetChannel();
	if (frame_channel == UNDEFINED_CHANNEL)
	{
		SetErrorText("Please select a channel for CMD_FS signal");
		return false;
	}

	Channel command_channel = mCommandChannelInterface->GetChannel();
	if (command_channel == UNDEFINED_CHANNEL)
	{
		SetErrorText("Please select a channel for CMD_D signal");
		return false;
	}

	Channel status_channel = mStatusChannelInterface->GetChannel();
	if (status_channel == UNDEFINED_CHANNEL)
	{
		SetErrorText("Please select a channel for STAT_D signal");
		return false;
	}

	if ((clock_channel == frame_channel) || (clock_channel == command_channel) || (frame_channel == command_channel)
		|| (clock_channel == status_channel) || (clock_channel == status_channel) || (frame_channel == status_channel))
	{
		SetErrorText("Please select different channels for the GSBus signals");
		return false;
	}

	mClockChannel = clock_channel;
	mFrameChannel = frame_channel;
	mCommandChannel = command_channel;
	mStatusChannel = status_channel;

	mBitsPerFrame = mBitsPerFrameInterface->GetNumber();
	mChannelsPerFrame = mChannelsPerFrameInterface->GetNumber();
	mDataBitsPerChannel = mDataBitsPerChannelInterface->GetNumber();
	mStatusBitsPerChannel = mStatusBitsPerChannelInterface->GetNumber();
	mParityBitsPerChannel = (mBitsPerFrame / mChannelsPerFrame) - mDataBitsPerChannel - mStatusBitsPerChannel;

	mShiftOrder = AnalyzerEnums::ShiftOrder(U32(mShiftOrderInterface->GetNumber()));
	mDataValidEdge = AnalyzerEnums::EdgeDirection(U32(mDataValidEdgeInterface->GetNumber()));
	mSigned = AnalyzerEnums::Sign(U32(mSignedInterface->GetNumber()));

	//AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );

	ClearChannels();
	AddChannel(mClockChannel, "CLOCK", true);
	AddChannel(mFrameChannel, "FRAME", true);
	AddChannel(mCommandChannel, "COMMAND", true);
	AddChannel(mStatusChannel, "STATUS", true);

	return true;
}

void GSBusAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mClockChannelInterface->SetChannel(mClockChannel);
	mFrameChannelInterface->SetChannel(mFrameChannel);
	mCommandChannelInterface->SetChannel(mCommandChannel);
	mStatusChannelInterface->SetChannel(mStatusChannel);

	mBitsPerFrameInterface->SetNumber(mBitsPerFrame);
	mChannelsPerFrameInterface->SetNumber(mChannelsPerFrame);
	mDataBitsPerChannelInterface->SetNumber(mDataBitsPerChannel);
	mStatusBitsPerChannelInterface->SetNumber(mStatusBitsPerChannel);

	mShiftOrderInterface->SetNumber(mShiftOrder);
	mDataValidEdgeInterface->SetNumber(mDataValidEdge);
	mSignedInterface->SetNumber(mSigned);
}

void GSBusAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString(settings);

	const char* name_string;	//the first thing in the archive is the name of the protocol analyzer that the data belongs to.
	text_archive >> &name_string;
	if (strcmp(name_string, "SaleaeGSBusAnalyzer") != 0)
		AnalyzerHelpers::Assert("SaleaeGSBusAnalyzer: Provided with a settings string that doesn't belong to us;");

	text_archive >> mClockChannel;
	text_archive >> mFrameChannel;
	text_archive >> mCommandChannel;
	text_archive >> mStatusChannel;

	text_archive >> *(U32*)&mBitsPerFrame;
	text_archive >> *(U32*)&mChannelsPerFrame;
	text_archive >> *(U32*)&mDataBitsPerChannel;
	text_archive >> *(U32*)&mStatusBitsPerChannel;
	text_archive >> *(U32*)&mParityBitsPerChannel;

	text_archive >> *(U32*)&mShiftOrder;
	text_archive >> *(U32*)&mDataValidEdge;

	//check to make sure loading it actual works befor assigning the result -- do this when adding settings to an anylzer which has been previously released.
	AnalyzerEnums::Sign sign;
	if (text_archive >> *(U32*)&sign)
		mSigned = sign;

	ClearChannels();
	AddChannel(mClockChannel, "CLOCK", true);
	AddChannel(mFrameChannel, "FRAME", true);
	AddChannel(mCommandChannel, "COMMAND", true);
	AddChannel(mStatusChannel, "STATUS", true);

	UpdateInterfacesFromSettings();
}

const char* GSBusAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << "SaleaeGSBusAnalyzer";

	text_archive << mClockChannel;
	text_archive << mFrameChannel;
	text_archive << mCommandChannel;
	text_archive << mStatusChannel;

	text_archive << mBitsPerFrame;
	text_archive << mChannelsPerFrame;
	text_archive << mDataBitsPerChannel;
	text_archive << mStatusBitsPerChannel;
	text_archive << mParityBitsPerChannel;

	text_archive << mShiftOrder;
	text_archive << mDataValidEdge;
	text_archive << mSigned;

	return SetReturnString(text_archive.GetString());
}