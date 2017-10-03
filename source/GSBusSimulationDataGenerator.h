#ifndef GSBUS_SIMULATION_DATA_GENERATOR
#define GSBUS_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <AnalyzerHelpers.h>
#include <string>

class GSBusAnalyzerSettings;

enum ChannelSelector { Ch1, Ch2, Ch3, Ch4, Ch5, Ch6, Ch7, Ch8 };
enum BitGenerarionState { Init, LeftPadding, Data, RightPadding };

class GSBusSimulationDataGenerator
{
public:
	GSBusSimulationDataGenerator();
	~GSBusSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, GSBusAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	GSBusAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

	SimulationChannelDescriptorGroup mSimulationChannels;
	SimulationChannelDescriptor* mClock;
	SimulationChannelDescriptor* mFrame;
	SimulationChannelDescriptor* mCommand;
	SimulationChannelDescriptor* mStatus;

protected: //GSBus specitic
	void InitSineWave();
	void WriteBit(BitState command, BitState status, BitState frame);
	S32 GetNextAudioWord();
	BitState GetNextCommandBit();
	BitState GetNextStatusBit();
	BitState GetNextFrameBit();

	std::vector<int> mSineWaveSamples1;
	std::vector<int> mSineWaveSamples2;
	std::vector<int> mSineWaveSamples3;
	std::vector<int> mSineWaveSamples4;
	std::vector<int> mSineWaveSamples5;
	std::vector<int> mSineWaveSamples6;
	std::vector<int> mSineWaveSamples7;
	std::vector<int> mSineWaveSamples8;

	ClockGenerator mClockGenerator;

	std::vector<BitState> mFrameBits;
	U32 mCurrentFrameBitIndex;

	std::vector<U32> mBitMasks;
	U32 mCurrentAudioWordIndex;

	ChannelSelector mCurrentAudioChannel;
	U32 mCurrentBitIndex;
	int mCurrentWord;
	U32 mPaddingCount;
	BitGenerarionState mBitGenerationState;

	//Fake data settings:
	double mAudioSampleRate;
	bool mUseShortFrames;
	U32 mNumPaddingBits;
};
#endif //GSBUS_SIMULATION_DATA_GENERATOR