#include "GSBusSimulationDataGenerator.h"
#include "GSBusAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

GSBusSimulationDataGenerator::GSBusSimulationDataGenerator()
  : mNumPaddingBits(0),
	mAudioSampleRate(24000000),
	mUseShortFrames(false)
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
	double bits_per_s = mAudioSampleRate * 2.0 * double(mSettings->mBitsPerWord + mNumPaddingBits);
	mClockGenerator.Init(bits_per_s, mSimulationSampleRateHz);

	mCurrentAudioWordIndex = 0;
	mCurrentAudioChannel = Ch1;
	mPaddingCount = 0;

	U32 audio_bit_depth = mSettings->mBitsPerWord;

	if (mSettings->mShiftOrder == AnalyzerEnums::MsbFirst)
	{
		U32 mask = 1 << (audio_bit_depth - 1);
		for (U32 i = 0; i<audio_bit_depth; i++)
		{
			mBitMasks.push_back(mask);
			mask = mask >> 1;
		}
	}
	else
	{
		U32 mask = 1;
		for (U32 i = 0; i<audio_bit_depth; i++)
		{
			mBitMasks.push_back(mask);
			mask = mask << 1;
		}
	}

	//enum PcmFrameType { FRAME_TRANSITION_TWICE_EVERY_WORD, FRAME_TRANSITION_ONCE_EVERY_WORD, FRAME_TRANSITION_TWICE_EVERY_FOUR_WORDS };
	U32 bits_per_word = audio_bit_depth + mNumPaddingBits;
	switch (mSettings->mFrameType)
	{
	case FRAME_TRANSITION_ONCE_EVERY_WORD:
		if (mUseShortFrames == false)
		{
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_LOW);
		}
		else
		{
			mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 1; i< (bits_per_word * 2); i++)
				mFrameBits.push_back(BIT_LOW);
		}
		break;
	case FRAME_TRANSITION_ONCE_EVERY_TWO_WORDS:
		if (mUseShortFrames == false)
		{
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_LOW);
		}
		else
		{
			mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 1; i< (bits_per_word * 4); i++)
				mFrameBits.push_back(BIT_LOW);
		}
		break;
	case FRAME_TRANSITION_ONCE_EVERY_FOUR_WORDS:
		if (mUseShortFrames == false)
		{
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_LOW);
		}
		else
		{
			mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 1; i< (bits_per_word * 8); i++)
				mFrameBits.push_back(BIT_LOW);
		}
		break;
	case FRAME_TRANSITION_ONCE_EVERY_EIGHT_WORDS:
		if (mUseShortFrames == false)
		{
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 0; i<bits_per_word; i++)
				mFrameBits.push_back(BIT_LOW);
		}
		else
		{
			mFrameBits.push_back(BIT_HIGH);
			for (U32 i = 1; i< (bits_per_word * 16); i++)
				mFrameBits.push_back(BIT_LOW);
		}
		break;
	default:
		AnalyzerHelpers::Assert("unexpected");
		break;
	}

	mCurrentWord = GetNextAudioWord();
	mCurrentBitIndex = 0;
	mCurrentFrameBitIndex = 0;
	mBitGenerationState = Init;
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

BitState GSBusSimulationDataGenerator::GetNextFrameBit()
{
	BitState bit_state = mFrameBits[mCurrentFrameBitIndex];
	mCurrentFrameBitIndex++;
	if (mCurrentFrameBitIndex >= mFrameBits.size())
		mCurrentFrameBitIndex = 0;
	return bit_state;
}

//enum BitGenerarionState { Init, LeftPadding, Data, RightPadding };
BitState GSBusSimulationDataGenerator::GetNextCommandBit()
{
	switch (mBitGenerationState)
	{
	case Init:
		if (mSettings->mBitAlignment == BITS_SHIFTED_RIGHT_1)
		{
			mBitGenerationState = LeftPadding;
			return BIT_LOW;  //just once, we'll insert a 1-bit offset.
		}
		else
		{
			mBitGenerationState = LeftPadding;
			return GetNextCommandBit();
		}
		break;
	case LeftPadding:
		if (mSettings->mWordAlignment == RIGHT_ALIGNED)
		{
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
		}
		else
		{
			mBitGenerationState = Data;
			return GetNextCommandBit();
		}
		break;
	case Data:
		if (mCurrentBitIndex == mSettings->mBitsPerWord)
		{
			mCurrentBitIndex = 0;
			mCurrentWord = GetNextAudioWord();
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
		if (mSettings->mWordAlignment == LEFT_ALIGNED)
		{
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
		}
		else
		{
			mBitGenerationState = LeftPadding;
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
		if (mSettings->mBitAlignment == BITS_SHIFTED_RIGHT_1)
		{
			mBitGenerationState = LeftPadding;
			return BIT_LOW;  //just once, we'll insert a 1-bit offset.
		}
		else
		{
			mBitGenerationState = LeftPadding;
			return GetNextStatusBit();
		}
		break;
	case LeftPadding:
		if (mSettings->mWordAlignment == RIGHT_ALIGNED)
		{
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
		}
		else
		{
			mBitGenerationState = Data;
			return GetNextStatusBit();
		}
		break;
	case Data:
		if (mCurrentBitIndex == mSettings->mBitsPerWord)
		{
			mCurrentBitIndex = 0;
			mCurrentWord = GetNextAudioWord();
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
		if (mSettings->mWordAlignment == LEFT_ALIGNED)
		{
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
		}
		else
		{
			mBitGenerationState = LeftPadding;
			return GetNextStatusBit();
		}
		break;
	default:
		AnalyzerHelpers::Assert("unexpected");
		return BIT_LOW; //eliminate warning
		break;
	}
}

S32 GSBusSimulationDataGenerator::GetNextAudioWord()
{
	S32 value;

	//return 0xFFFF;

	if (mCurrentAudioChannel == Ch1)
	{
		value = mSineWaveSamples1[mCurrentAudioWordIndex];
		mCurrentAudioChannel = Ch2;
	}
	else
	{
		value = mSineWaveSamples2[mCurrentAudioWordIndex];
		mCurrentAudioChannel = Ch1;
		mCurrentAudioWordIndex++;
		if (mCurrentAudioWordIndex >= mSineWaveSamples1.size())
			mCurrentAudioWordIndex = 0;
	}

	return value;
}

void GSBusSimulationDataGenerator::InitSineWave()
{
	U32 sine_freq = 220;
	U32 samples_for_one_cycle = U32(mAudioSampleRate / double(sine_freq));
	int max_amplitude = (1 << (mSettings->mBitsPerWord - 2)) - 1;

	mSineWaveSamples1.reserve(samples_for_one_cycle);
	mSineWaveSamples2.reserve(samples_for_one_cycle);
	for (U32 i = 0; i<samples_for_one_cycle; i++)
	{
		double t = double(i) / double(samples_for_one_cycle);
		double val_right = sin(t * 6.28318530718);
		double val_left = sin(t * 6.28318530718 * 2.0);
		mSineWaveSamples1.push_back(int(double(max_amplitude) * val_right));
		mSineWaveSamples2.push_back(int(double(max_amplitude) * val_left));
	}
}

inline void GSBusSimulationDataGenerator::WriteBit(BitState command, BitState status, BitState frame)
{
	//start 'low', pause 1/2 period:
	mSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(1.0));

	//'posedge' on clock, write update data lines:
	mClock->Transition();

	mFrame->TransitionIfNeeded(frame);

	mCommand->TransitionIfNeeded(command);

	mCommand->TransitionIfNeeded(status);

	mSimulationChannels.AdvanceAll(mClockGenerator.AdvanceByHalfPeriod(1.0));

	//'negedge' on clock, data is valid.
	mClock->Transition();
}