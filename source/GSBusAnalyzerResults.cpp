#include "GSBusAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "GSBusAnalyzer.h"
#include "GSBusAnalyzerSettings.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <cstring>

GSBusAnalyzerResults::GSBusAnalyzerResults( GSBusAnalyzer* analyzer, GSBusAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

GSBusAnalyzerResults::~GSBusAnalyzerResults()
{
}

void GSBusAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)
{
	ClearResultStrings();
	Frame frame = GetFrame(frame_index);

	// Convert the frame type number to string (char array) representation.
	char channel_str[128];
	U64 channel_number = frame.mType;
	std::stringstream tss;
	tss << channel_number;
	strcpy(channel_str, tss.str().c_str());

	// A frame type number above 200 means an error.
	if (frame.mType <= 200)
	{
		if (channel == mSettings->mCommandChannel)
		{
			// Command data.
			char command_str[128];

			if ((display_base == Decimal) && (mSettings->mSigned == AnalyzerEnums::SignedInteger))
			{
				S64 signed_number = AnalyzerHelpers::ConvertToSignedNumber(frame.mData1, mSettings->mDataBitsPerChannel);
				std::stringstream ss;
				ss << signed_number;
				strcpy(command_str, ss.str().c_str());
			}
			else
			{
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, mSettings->mDataBitsPerChannel, command_str, 128);
			}

			AddResultString("Ch ", channel_str, ": ", command_str);
		}
		
		if (channel == mSettings->mStatusChannel)
		{
			// Status data.
			char status_str[128];

			if ((display_base == Decimal) && (mSettings->mSigned == AnalyzerEnums::SignedInteger))
			{
				S64 signed_number = AnalyzerHelpers::ConvertToSignedNumber(frame.mData2, mSettings->mDataBitsPerChannel);
				std::stringstream ss;
				ss << signed_number;
				strcpy(status_str, ss.str().c_str());
			}
			else
			{
				AnalyzerHelpers::GetNumberString(frame.mData2, display_base, mSettings->mDataBitsPerChannel, status_str, 128);
			}

			AddResultString("Ch ", channel_str, ": ", status_str);
		}
	}
	else
	{ 
		// Check for frame types corresponding to errors.
		if (frame.mType == 255)
		{
			AddResultString("!");
			AddResultString("Error");
			AddResultString("Error: bits don't divide evenly");
			AddResultString("Error: bits don't divide evenly between channels");
		}

		if (frame.mType == 254)
		{
			char bits_per_frame[32];
			sprintf(bits_per_frame, "%d", mSettings->mBitsPerFrame);

			AddResultString("!");
			AddResultString("Error");
			AddResultString("Error: too few bits");
			AddResultString("Error: too few bits in the frame, expecting ", bits_per_frame);
		}
	}
}

void GSBusAnalyzerResults::GenerateExportFile(const char* file, DisplayBase display_base, U32 /*export_type_user_id*/)
{
	std::stringstream ss;
	void* f = AnalyzerHelpers::StartFile(file);

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	ss << "Time [s],Channel,Command Value,Status Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for (U64 i = 0; i < num_frames; i++)
	{
		Frame frame = GetFrame(i);

		// Convert the frame type number to string (char array) representation.
		char channel_str[128];
		U64 channel_number = frame.mType;
		std::stringstream tss;
		tss << channel_number;
		strcpy(channel_str, tss.str().c_str());

		if (frame.mType <= 200)
		{
			// Command data.
			char time_str[128];
			AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128);

			char command_str[128];
			if ((display_base == Decimal) && (mSettings->mSigned == AnalyzerEnums::SignedInteger))
			{
				S64 signed_number = AnalyzerHelpers::ConvertToSignedNumber(frame.mData1, mSettings->mDataBitsPerChannel);
				std::stringstream nss;
				nss << signed_number;
				strcpy(command_str, nss.str().c_str());
			}
			else
			{
				AnalyzerHelpers::GetNumberString(frame.mData1, display_base, mSettings->mDataBitsPerChannel, command_str, 128);
			}

			// Status data.
			char status_str[128];
			if ((display_base == Decimal) && (mSettings->mSigned == AnalyzerEnums::SignedInteger))
			{
				S64 signed_number = AnalyzerHelpers::ConvertToSignedNumber(frame.mData2, mSettings->mDataBitsPerChannel);
				std::stringstream nss;
				nss << signed_number;
				strcpy(status_str, nss.str().c_str());
			}
			else
			{
				AnalyzerHelpers::GetNumberString(frame.mData2, display_base, mSettings->mDataBitsPerChannel, status_str, 128);
			}

			ss << time_str << "," << channel_str << "," << command_str << "," << status_str << std::endl;
		}

		AnalyzerHelpers::AppendToFile((U8*)ss.str().c_str(), ss.str().length(), f);
		ss.str(std::string());

		if (UpdateExportProgressAndCheckForCancel(i, num_frames) == true)
		{
			AnalyzerHelpers::EndFile(f);
			return;
		}
	}

	UpdateExportProgressAndCheckForCancel(num_frames, num_frames);
	AnalyzerHelpers::EndFile(f);
}

void GSBusAnalyzerResults::GenerateFrameTabularText(U64 frame_index, DisplayBase display_base)
{
	ClearTabularText();

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	Frame frame = GetFrame(frame_index);

	// Convert the frame type number to string (char array) representation.
	char channel_str[128];
	U64 channel_number = frame.mType;
	std::stringstream tss;
	tss << channel_number;
	strcpy(channel_str, tss.str().c_str());

	if (frame.mType <= 200)
	{
		char time_str[128];
		AnalyzerHelpers::GetTimeString(frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128);

		// Command data.
		char command_str[128];
		if ((display_base == Decimal) && (mSettings->mSigned == AnalyzerEnums::SignedInteger))
		{
			S64 signed_number = AnalyzerHelpers::ConvertToSignedNumber(frame.mData1, mSettings->mDataBitsPerChannel);
			std::stringstream nss;
			nss << signed_number;
			strcpy(command_str, nss.str().c_str());
		}
		else
		{
			AnalyzerHelpers::GetNumberString(frame.mData1, display_base, mSettings->mDataBitsPerChannel, command_str, 128);
		}

		// Status data.
		char status_str[128];
		if ((display_base == Decimal) && (mSettings->mSigned == AnalyzerEnums::SignedInteger))
		{
			S64 signed_number = AnalyzerHelpers::ConvertToSignedNumber(frame.mData2, mSettings->mDataBitsPerChannel);
			std::stringstream nss;
			nss << signed_number;
			strcpy(status_str, nss.str().c_str());
		}
		else
		{
			AnalyzerHelpers::GetNumberString(frame.mData2, display_base, mSettings->mDataBitsPerChannel, status_str, 128);
		}

		AddTabularText(time_str, channel_str, command_str, status_str);
	}
	else
	{
		if (frame.mType == 255)
		{
			AddTabularText("Error: bits don't divide evenly between channels");
		}
	}
}

void GSBusAnalyzerResults::GeneratePacketTabularText(U64 /*packet_id*/, DisplayBase /*display_base*/)  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}

void GSBusAnalyzerResults::GenerateTransactionTabularText(U64 /*transaction_id*/, DisplayBase /*display_base*/)  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString("not supported");
}