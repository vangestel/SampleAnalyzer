#include "GSBusSimulationDataGenerator.h"
#include "GSBusAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

GSBusSimulationDataGenerator::GSBusSimulationDataGenerator()
  : mNumPaddingBits(0),
	mSampleRate(48000000)
{
}

GSBusSimulationDataGenerator::~GSBusSimulationDataGenerator()
{
}

void GSBusSimulationDataGenerator::Initialize( U32 simulation_sample_rate, GSBusAnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	if (mSettings->mDataValidEdge == AnalyzerEnums::NegEdge)
		mClock = mSimulationChannels.Add(mSettings->mClockChannel, mSimulationSampleRateHz, BIT_LOW);
	else
		mClock = mSimulationChannels.Add(mSettings->mClockChannel, mSimulationSampleRateHz, BIT_HIGH);

	mFrame = mSimulationChannels.Add(mSettings->mFrameChannel, mSimulationSampleRateHz, BIT_LOW);
	mCommand = mSimulationChannels.Add(mSettings->mCommandChannel, mSimulationSampleRateHz, BIT_LOW);
	mStatus = mSimulationChannels.Add(mSettings->mStatusChannel, mSimulationSampleRateHz, BIT_LOW);

	InitSineWave();
	double bits_per_s = 48000000;
	mClockGenerator.Init(bits_per_s, mSimulationSampleRateHz);

	mCurrentWordIndex = 0;
	mCurrentChannel = 0;
	mPaddingCount = 0;

	U32 data_bit_depth = mSettings->mDataBitsPerChannel;

	if (mSettings->mShiftOrder == AnalyzerEnums::MsbFirst)
	{
		U32 mask = 1 << (data_bit_depth - 1);
		for (U32 i = 0; i < data_bit_depth; i++)
		{
			mBitMasks.push_back(mask);
			mask = mask >> 1;
		}
	}
	else
	{
		U32 mask = 1;
		for (U32 i = 0; i < data_bit_depth; i++)
		{
			mBitMasks.push_back(mask);
			mask = mask << 1;
		}
	}

	U32 bits_per_word = mSettings->mBitsPerFrame / mSettings->mChannelsPerFrame;

	for (U32 i = 0; i < mSettings->mBitsPerFrame - 1; i++)
	{
		mFrameBits.push_back(BIT_LOW);
	}
	mFrameBits.push_back(BIT_HIGH);

	mCurrentWord = GetNextDataWord();
	mCurrentBitIndex = 0;
	mCurrentFrameBitIndex = 0;
	mBitGenerationState = Init;
}

void GSBusSimulationDataGenerator::InitSineWave()
{
	U32 sine_freq = 2000;
	U32 samples_for_one_cycle = U32(mSampleRate / double(sine_freq));
	int max_amplitude = (1 << (mSettings->mDataBitsPerChannel - 2)) - 1;

	mSineWaveSamples.reserve(samples_for_one_cycle);
	for (U32 i = 0; i < samples_for_one_cycle; i++)
	{
		double t = double(i) / double(samples_for_one_cycle);
		double val_right = sin(t * 6.28318530718);
		double val_left = sin(t * 6.28318530718 * 2.0);
		mSineWaveSamples.push_back(int(double(max_amplitude) * val_right));
	}
}

S32 GSBusSimulationDataGenerator::GetNextDataWord()
{
	S32 value;

	//return 0xFFFF;

	if (mCurrentChannel < mSettings->mChannelsPerFrame)
	{
		value = mSineWaveSamples[mCurrentWordIndex];
		mCurrentChannel++;
	}
	else
	{
		value = mSineWaveSamples[mCurrentWordIndex];
		mCurrentChannel = 0;
		mCurrentWordIndex++;
		if (mCurrentWordIndex >= mSineWaveSamples.size())
			mCurrentWordIndex = 0;
	}

	return value;
}

U32 GSBusSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while (mCommand->GetCurrentSampleNumber() < adjusted_largest_sample_requested)
	{
		WriteBit(GetNextCommandBit(), GetNextStatusBit(), GetNextFrameBit());
	}

	*simulation_channels = mSimulationChannels.GetArray();
	return mSimulationChannels.GetCount();
}

inline void GSBusSimulationDataGenerator::WriteBit(BitState command, BitState status, BitState frame)
{
	//start 'low', pause 1/2 period:
	mSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(1.0));

	//'posedge' on clock, write update data lines:
	mClock->Transition();

	mFrame->TransitionIfNeeded(frame);

	mCommand->TransitionIfNeeded(command);

	mStatus->TransitionIfNeeded(status);

	mSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(1.0));

	//'negedge' on clock, data is valid.
	mClock->Transition();
}

BitState GSBusSimulationDataGenerator::GetNextFrameBit()
{
	BitState bit_state = mFrameBits[mCurrentFrameBitIndex];

	mCurrentFrameBitIndex++;

	if (mCurrentFrameBitIndex >= mFrameBits.size())
		mCurrentFrameBitIndex = 0;

	return bit_state;
}

BitState GSBusSimulationDataGenerator::GetNextCommandBit()
{
	switch (mBitGenerationState)
	{
	case Init:
		mBitGenerationState = LeftPadding;
		return GetNextCommandBit();
		break;
	case LeftPadding:
		mBitGenerationState = Data;
		return GetNextCommandBit();
		break;
	case Data:
		if (mCurrentBitIndex == mSettings->mDataBitsPerChannel)
		{
			mCurrentBitIndex = 0;
			mCurrentWord = GetNextDataWord();
			mBitGenerationState = RightPadding;
			return GetNextCommandBit();
		}
		else
		{
			BitState bit_state;

			if ((mCurrentWord & mBitMasks[mCurrentBitIndex]) == 0)
				bit_state = BIT_LOW;
			else
				bit_state = BIT_HIGH;

			mCurrentBitIndex++;
			return bit_state;
		}
		break;
	case RightPadding:
		if (mPaddingCount < mNumPaddingBits)
		{
			mPaddingCount++;
			return BIT_LOW;
		}
		else
		{
			mBitGenerationState = Data;
			mPaddingCount = 0;
			return GetNextCommandBit();
		}
		break;
	default:
		AnalyzerHelpers::Assert("unexpected");
		return BIT_LOW; //eliminate warning
		break;
	}
}

BitState GSBusSimulationDataGenerator::GetNextStatusBit()
{
	switch (mBitGenerationState)
	{
	case Init:
		mBitGenerationState = LeftPadding;
		return GetNextStatusBit();
		break;
	case LeftPadding:
		mBitGenerationState = Data;
		return GetNextStatusBit();
		break;
	case Data:
		if (mCurrentBitIndex == mSettings->mDataBitsPerChannel)
		{
			mCurrentBitIndex = 0;
			mCurrentWord = GetNextDataWord();
			mBitGenerationState = RightPadding;
			return GetNextStatusBit();
		}
		else
		{
			BitState bit_state;

			if ((mCurrentWord & mBitMasks[mCurrentBitIndex]) == 0)
				bit_state = BIT_LOW;
			else
				bit_state = BIT_HIGH;

			mCurrentBitIndex++;
			return bit_state;
		}
		break;
	case RightPadding:
		if (mPaddingCount < mNumPaddingBits)
		{
			mPaddingCount++;
			return BIT_LOW;
		}
		else
		{
			mBitGenerationState = Data;
			mPaddingCount = 0;
			return GetNextStatusBit();
		}
		break;
	default:
		AnalyzerHelpers::Assert("unexpected");
		return BIT_LOW; //eliminate warning
		break;
	}
}