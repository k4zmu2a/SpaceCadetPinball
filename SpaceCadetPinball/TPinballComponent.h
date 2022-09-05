#pragma once

struct zmap_header_type;
struct gdrv_bitmap8;
struct render_sprite_type_struct;
struct component_control;
struct vector2;
class TPinballTable;


enum class MessageCode
{
	// Private codes <1000, different meaning for each component
	TFlipperExtend = 1,
	TFlipperRetract = 2,
	
	TLightTurnOff = 0,
	TLightTurnOn = 1,
	TLightGetLightOnFlag = 2,
	TLightGetFlasherOnFlag = 3,
	TLightFlasherStart = 4,
	TLightApplyMultDelay = 5,
	TLightApplyDelay = 6,
	TLightFlasherStartTimed = 7,
	TLightTurnOffTimed = 8,
	TLightTurnOnTimed = 9,
	TLightSetOnStateBmpIndex = 11,
	TLightIncOnStateBmpIndex = 12,
	TLightDecOnStateBmpIndex = 13,
	TLightResetTimed = 14,
	TLightFlasherStartTimedThenStayOn = 15,
	TLightFlasherStartTimedThenStayOff = 16,
	TLightToggleValue = 17,
	TLightResetAndToggleValue = 18,
	TLightResetAndTurnOn = 19,
	TLightResetAndTurnOff = 20,
	TLightToggle = 21,
	TLightResetAndToggle = 22,
	TLightSetMessageField = 23,
	TLightFtTmpOverrideOn = -24,
	TLightFtTmpOverrideOff = -25,
	TLightFtResetOverride = -26,

	TLightGroupStepBackward = 24,
	TLightGroupStepForward = 25,
	TLightGroupAnimationBackward = 26,
	TLightGroupAnimationForward = 27,
	TLightGroupRandomAnimation1 = 28,
	TLightGroupRandomAnimation2 = 29,
	TLightGroupRandomAnimationSaturation = 30,
	TLightGroupRandomAnimationDesaturation = 31,
	TLightGroupOffsetAnimationForward = 32,
	TLightGroupOffsetAnimationBackward = 33,
	TLightGroupReset = 34,
	TLightGroupTurnOnAtIndex = 35,
	TLightGroupTurnOffAtIndex = 36,
	TLightGroupGetOnCount = 37,
	TLightGroupGetLightCount = 38,
	TLightGroupGetMessage2 = 39,
	TLightGroupGetAnimationFlag = 40,
	TLightGroupResetAndTurnOn = 41,
	TLightGroupResetAndTurnOff = 42,
	TLightGroupRestartNotifyTimer = 43,
	TLightGroupFlashWhenOn = 44,
	TLightGroupToggleSplitIndex = 45,
	TLightGroupStartFlasher = 46,

	// Public codes 1000+, apply to all components
	LeftFlipperInputPressed = 1000,
	LeftFlipperInputReleased = 1001,
	RightFlipperInputPressed = 1002,
	RightFlipperInputReleased = 1003,
	PlungerInputPressed = 1004,
	PlungerInputReleased = 1005,
	Pause = 1008,
	Resume = 1009,
	LooseFocus = 1010,
	SetTiltLock = 1011,
	ResetTiltLock = 1012,
	StartGamePlayer1 = 1013,
	NewGame = 1014,
	PlungerFeedBall = 1015,
	PlungerStartFeedTimer = 1016,
	PlungerLaunchBall = 1017,
	PlungerRelaunchBall = 1018,
	PlayerChanged = 1020,
	SwitchToNextPlayer = 1021,
	GameOver = 1022,
	Reset = 1024,
};

// Temporary hacks for int -> enum class migration.
template <typename T, typename X = typename std::underlying_type<T>::type>
constexpr typename std::enable_if<std::is_enum<T>::value, X>::type operator~(T value)
{
	return static_cast<X>(value);
}
#if defined(__GNUC__) || defined(__clang__)
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#define DEPRECATED
#endif


class TPinballComponent
{
public:
	TPinballComponent(TPinballTable* table, int groupIndex, bool loadVisuals);
	virtual ~TPinballComponent();
	virtual int Message(int code, float value);
	virtual int Message2(MessageCode code, float value)
	{
		return Message(~code, value);
	}
	virtual void port_draw();
	int get_scoring(unsigned int index) const;
	virtual vector2 get_coordinates();

	char UnusedBaseFlag;
	char ActiveFlag;
	int MessageField;
	char* GroupName;
	component_control* Control;
	int GroupIndex;
	render_sprite_type_struct* RenderSprite;
	TPinballTable* PinballTable;
	std::vector<gdrv_bitmap8*>* ListBitmap;
	std::vector<zmap_header_type*>* ListZMap;
private:
	float VisualPosNormX;
	float VisualPosNormY;
};


class TPinballComponent2 : public TPinballComponent
{
public:
	TPinballComponent2(TPinballTable* table, int group_index, bool load_visuals)
		: TPinballComponent(table, group_index, load_visuals)
	{
	}

	DEPRECATED int Message(int code, float value) override
	{
		return Message2(static_cast<MessageCode>(code), value);
	}
};
