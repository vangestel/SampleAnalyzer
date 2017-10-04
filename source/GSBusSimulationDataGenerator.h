#ifndef GSBUS_SIMULATION_DATA_GENERATOR
#define GSBUS_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <AnalyzerHelpers.h>
#include <string>

class GSBusAnalyzerSettings;

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
	S32 GetNextDataWord();
	BitState GetNextCommandBit();
	BitState GetNextStatusBit();
	BitState GetNextFrameBit();

	std::vector<int> mSineWaveSamples;

	ClockGenerator mClockGenerator;

	std::vector<BitState> mFrameBits;
	U32 mCurrentFrameBitIndex;

	std::vector<U32> mBitMasks;
	U32 mCurrentWordIndex;
	U32 mCurrentChannel;
	U32 mCurrentBitIndex;
	int mCurrentWord;
	U32 mPaddingCount;
	BitGenerarionState mBitGenerationState;

	//Fake data settings:
	double mSampleRate;
	U32 mNumPaddingBits;
};
#endif //GSBUS_SIMULATION_DATA_GENERATOR