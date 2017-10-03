#ifndef GSBUS_ANALYZER_SETTINGS
#define GSBUS_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

enum PcmFrameType { FRAME_TRANSITION_ONCE_EVERY_WORD, FRAME_TRANSITION_ONCE_EVERY_TWO_WORDS, FRAME_TRANSITION_ONCE_EVERY_FOUR_WORDS, FRAME_TRANSITION_ONCE_EVERY_EIGHT_WORDS };
enum PcmWordAlignment { LEFT_ALIGNED, RIGHT_ALIGNED };
enum PcmBitAlignment { BITS_SHIFTED_RIGHT_1, NO_SHIFT };
enum PcmWordSelectInverted { WS_INVERTED, WS_NOT_INVERTED };

class GSBusAnalyzerSettings : public AnalyzerSettings
{
public:
	GSBusAnalyzerSettings();
	virtual ~GSBusAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	Channel mClockChannel;
	Channel mFrameChannel;
	Channel mCommandChannel;
	Channel mStatusChannel;

	AnalyzerEnums::ShiftOrder mShiftOrder;
	AnalyzerEnums::EdgeDirection mDataValidEdge;
	U32 mBitsPerWord;

	PcmWordAlignment mWordAlignment;
	PcmFrameType mFrameType;
	PcmBitAlignment mBitAlignment;
	AnalyzerEnums::Sign mSigned;


	PcmWordSelectInverted mWordSelectInverted;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mClockChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mFrameChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mCommandChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceChannel > mStatusChannelInterface;

	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mShiftOrderInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mDataValidEdgeInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mBitsPerWordInterface;

	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mFrameTypeInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mWordAlignmentInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mBitAlignmentInterface;

	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mSignedInterface;

	std::auto_ptr< AnalyzerSettingInterfaceNumberList > mWordSelectInvertedInterface;
};

#endif //GSBUS_ANALYZER_SETTINGS
