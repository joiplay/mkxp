/*
** input.cpp
**
** This file is part of mkxp.
**
** Copyright (C) 2013 Jonas Kulla <Nyocurio@gmail.com>
**
** mkxp is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** mkxp is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with mkxp.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "input.h"
#include "sharedstate.h"
#include "eventthread.h"
#include "keybindings.h"
#include "exception.h"
#include "util.h"

#include <SDL_scancode.h>
#include <SDL_mouse.h>

#include <vector>
#include <map>
#include <string.h>
#include <assert.h>
#include <math.h>

#define BUTTON_CODE_COUNT 24

// Map of Windows virtualkey codes to SDL scancodes.
// Used for pressex, triggerex and repeatex
#define m(vk,sc) { vk, SDL_SCANCODE_##sc }
std::map<int, int> vKeyToScancode{
    // 0x01 LEFT MOUSE
    // 0x02 RIGHT MOUSE
    m(0x03, CANCEL),
    // 0x04 MIDDLE MOUSE
    // 0x05 XBUTTON 1
    // 0x06 XBUTTON 2
    // 0x07 undef
    m(0x08, BACKSPACE),
    m(0x09, TAB),
    // 0x0a reserved
    // 0x0b reserved
    m(0x0c, CLEAR),
    m(0x0d, RETURN),
    // 0x0e undefined
    // 0x0f undefined
    // 0x10 SHIFT (both)
    // 0x11 CONTROL (both)
    // 0x12 ALT (both)
    m(0x13, PAUSE),
    m(0x14, CAPSLOCK),
    // 0x15 KANA, HANGUL
    // 0x16 undefined
    // 0x17 JUNJA
    // 0x18 FINAL
    // 0x19 HANJA, KANJI
    // 0x1a undefined
    m(0x1b, ESCAPE),
    // 0x1c CONVERT
    // 0x1d NONCONVERT
    // 0x1e ACCEPT
    // 0x1f MODECHANGE
    m(0x20, SPACE),
    m(0x21, PAGEUP),
    m(0x22, PAGEDOWN),
    m(0x23, END),
    m(0x24, HOME),
    m(0x25, LEFT),
    m(0x26, UP),
    m(0x27, RIGHT),
    m(0x28, DOWN),
    m(0x29, SELECT),
    // 0x2A print
    m(0x2b, EXECUTE),
    m(0x2c, PRINTSCREEN),
    m(0x2d, INSERT),
    m(0x2e, DELETE),
    m(0x2f, HELP),
    m(0x30, 0), m(0x31, 1),
    m(0x32, 2), m(0x33, 3),
    m(0x34, 4), m(0x35, 5),
    m(0x36, 6), m(0x37, 7),
    m(0x38, 8), m(0x39, 9),
    // 0x3a-0x40 undefined
    m(0x41, A), m(0x42, B),
    m(0x43, C), m(0x44, D),
    m(0x45, E), m(0x46, F),
    m(0x47, G), m(0x48, H),
    m(0x49, I), m(0x4a, J),
    m(0x4b, K), m(0x4c, L),
    m(0x4d, M), m(0x4e, N),
    m(0x4f, O), m(0x50, P),
    m(0x51, Q), m(0x52, R),
    m(0x53, S), m(0x54, T),
    m(0x55, U), m(0x56, V),
    m(0x57, W), m(0x58, X),
    m(0x59, Y), m(0x5a, Z),
    m(0x5b, LGUI), m(0x5c, RGUI),
    m(0x5d, APPLICATION),
    // 0x5e reserved
    m(0x5f, SLEEP),
    m(0x60, KP_0), m(0x61, KP_1),
    m(0x62, KP_2), m(0x63, KP_3),
    m(0x64, KP_4), m(0x65, KP_5),
    m(0x66, KP_6), m(0x67, KP_7),
    m(0x68, KP_8), m(0x69, KP_9),
    m(0x6a, KP_MULTIPLY),
    m(0x6b, KP_PLUS),
    m(0x6c, RETURN), // SEPARATOR
    m(0x6d, KP_MINUS),
    m(0x6e, KP_DECIMAL),
    m(0x6f, KP_DIVIDE),
    m(0x70, F1), m(0x71, F2),
    m(0x72, F3), m(0x73, F4),
    m(0x74, F5), m(0x75, F6),
    m(0x76, F7), m(0x77, F8),
    m(0x78, F9), m(0x79, F10),
    m(0x7a, F11), m(0x7b, F12),
    m(0x7c, F13), m(0x7d, F14),
    m(0x7e, F15), m(0x7f, F16),
    m(0x80, F17), m(0x81, F18),
    m(0x82, F19), m(0x83, F20),
    m(0x84, F21), m(0x85, F22),
    m(0x86, F23), m(0x87, F24),
    // 0x88-0x8f unassigned
    m(0x90, NUMLOCKCLEAR),
    m(0x91, SCROLLLOCK),
    // 0x92-0x96 oem specific
    // 0x97-0x9f unassigned
    m(0xa0, LSHIFT),
    m(0xa1, RSHIFT),
    m(0xa2, LCTRL),
    m(0xa3, RCTRL),
    m(0xa4, LALT),
    m(0xa5, RALT),
    m(0xa6, AC_BACK),
    m(0xa7, AC_FORWARD),
    m(0xa8, AC_REFRESH),
    m(0xa9, AC_STOP),
    m(0xaa, AC_SEARCH),
    // 0xab BROWSER_FAVORITES
    m(0xac, AC_HOME),
    m(0xad, AUDIOMUTE),
    m(0xae, VOLUMEDOWN),
    m(0xaf, VOLUMEUP),
    m(0xb0, AUDIONEXT),
    m(0xb1, AUDIOPREV),
    m(0xb2, AUDIOSTOP),
    m(0xb3, AUDIOPLAY),
    m(0xb4, MAIL),
    m(0xb5, MEDIASELECT),
    // 0xb6 LAUNCH_APP1
    // 0xb7 LAUNCH_APP2
    // 0xb8-0xb9 reserved
    
    // Everything below here is OEM
    // and can vary by country
    m(0xba, SEMICOLON),
    m(0xbb, EQUALS),
    m(0xbc, COMMA),
    m(0xbd, MINUS),
    m(0xbe, PERIOD),
    m(0xbf, SLASH),
    m(0xc0, GRAVE),
    // 0xc1-0xd7 reserved
    // 0xd8-0xda unassigned
    m(0xdb, LEFTBRACKET),
    m(0xdc, BACKSLASH),
    m(0xdd, RIGHTBRACKET),
    m(0xde, APOSTROPHE),
    // 0xdf OEM_8
    // 0xe0 reserved
    // 0xe1 oem-specific
    // 0xe2 OEM_102
    // 0xe3-0xe4 oem-specific
    // 0xe5 PROCESSKEY
    // 0xe6 oem-specific
    // 0xe7 PACKET
    // 0xe8 unassigned
    // 0xe9-0xf5 oem_specific
    // 0xf6 ATTN
    m(0xf7, CRSEL),
    m(0xf8, EXSEL),
    // 0xf9 EREOF
    m(0xfa, AUDIOPLAY), // PLAY, guessing
    // 0xfb ZOOM
    // 0xfc NONAME
    // 0xfd PA1
    m(0xfe, CLEAR)
};
#undef m

struct ButtonState
{
	bool pressed;
	bool triggered;
	bool repeated;

	ButtonState()
		: pressed(false),
		  triggered(false),
		  repeated(false)
	{}
};

struct KbBindingData
{
	SDL_Scancode source;
	Input::ButtonCode target;
};

struct Binding
{
	Binding(Input::ButtonCode target = Input::None)
		: target(target)
	{}

	virtual bool sourceActive() const = 0;
	virtual bool sourceRepeatable() const = 0;

	Input::ButtonCode target;
};

/* Keyboard binding */
struct KbBinding : public Binding
{
	KbBinding() {}

	KbBinding(const KbBindingData &data)
		: Binding(data.target),
		  source(data.source)
	{}

	bool sourceActive() const
	{
		/* Special case aliases */
		if (source == SDL_SCANCODE_LSHIFT)
			return EventThread::keyStates[source]
			    || EventThread::keyStates[SDL_SCANCODE_RSHIFT];

		if (source == SDL_SCANCODE_RETURN)
			return EventThread::keyStates[source]
			    || EventThread::keyStates[SDL_SCANCODE_KP_ENTER];

		return EventThread::keyStates[source];
	}

	bool sourceRepeatable() const
	{
		return (source >= SDL_SCANCODE_A     && source <= SDL_SCANCODE_0)    ||
		       (source >= SDL_SCANCODE_RIGHT && source <= SDL_SCANCODE_UP)   ||
		       (source >= SDL_SCANCODE_F1    && source <= SDL_SCANCODE_F12);
	}

	SDL_Scancode source;
};

/* Joystick button binding */
struct JsButtonBinding : public Binding
{
	JsButtonBinding() {}

	bool sourceActive() const
	{
		return EventThread::joyState.buttons[source];
	}

	bool sourceRepeatable() const
	{
		return true;
	}

	uint8_t source;
};

/* Joystick axis binding */
struct JsAxisBinding : public Binding
{
	JsAxisBinding() {}

	JsAxisBinding(uint8_t source,
	              AxisDir dir,
	              Input::ButtonCode target)
	    : Binding(target),
	      source(source),
	      dir(dir)
	{}

	bool sourceActive() const
	{
		int val = EventThread::joyState.axes[source];

		if (dir == Negative)
			return val < -JAXIS_THRESHOLD;
		else /* dir == Positive */
			return val > JAXIS_THRESHOLD;
	}

	bool sourceRepeatable() const
	{
		return true;
	}

	uint8_t source;
	AxisDir dir;
};

/* Joystick hat binding */
struct JsHatBinding : public Binding
{
	JsHatBinding() {}

	JsHatBinding(uint8_t source,
	              uint8_t pos,
	              Input::ButtonCode target)
	    : Binding(target),
	      source(source),
	      pos(pos)
	{}

	bool sourceActive() const
	{
		/* For a diagonal input accept it as an input for both the axes */
		return (pos & EventThread::joyState.hats[source]) != 0;
	}

	bool sourceRepeatable() const
	{
		return true;
	}

	uint8_t source;
	uint8_t pos;
};

/* Mouse button binding */
struct MsBinding : public Binding
{
	MsBinding() {}

	MsBinding(int buttonIndex,
	          Input::ButtonCode target)
	    : Binding(target),
	      index(buttonIndex)
	{}

	bool sourceActive() const
	{
#ifdef __ANDROID__
		if (index == 1 && EventThread::touchState.ignoreMouse == false)
		{
			for (size_t i = 0; i < MAX_FINGERS; ++i)
			{
				if (EventThread::touchState.fingers[i].down) return true;
			}
		}
#endif		
		return EventThread::mouseState.buttons[index];
	}

	bool sourceRepeatable() const
	{
		return false;
	}

	int index;
};

static Input::ButtonCode bcFromString(const std::string &str)
{
#define BC_CASE(button) \
	else if (str == #button) \
		return Input:: button

	if (false) {}
	BC_CASE(Down);
	BC_CASE(Left);
	BC_CASE(Right);
	BC_CASE(Up);
	BC_CASE(A);
	BC_CASE(B);
	BC_CASE(C);
	BC_CASE(X);
	BC_CASE(Y);
	BC_CASE(Z);
	BC_CASE(L);
	BC_CASE(R);

	return Input::None;
}

/* Not rebindable */
static const KbBindingData staticKbBindings[] =
{
	{ SDL_SCANCODE_LSHIFT, Input::Shift },
	{ SDL_SCANCODE_RSHIFT, Input::Shift },
	{ SDL_SCANCODE_LCTRL,  Input::Ctrl  },
	{ SDL_SCANCODE_RCTRL,  Input::Ctrl  },
	{ SDL_SCANCODE_LALT,   Input::Alt   },
	{ SDL_SCANCODE_RALT,   Input::Alt   },
	{ SDL_SCANCODE_F5,     Input::F5    },
	{ SDL_SCANCODE_F6,     Input::F6    },
	{ SDL_SCANCODE_F7,     Input::F7    },
	{ SDL_SCANCODE_F8,     Input::F8    },
	{ SDL_SCANCODE_F9,     Input::F9    }
};

static elementsN(staticKbBindings);

/* Maps ButtonCode enum values to indices
 * in the button state array */
static const int mapToIndex[] =
{
	0, 0,
	1, 0, 2, 0, 3, 0, 4, 0,
	0,
	5, 6, 7, 8, 9, 10, 11, 12,
	0, 0,
	13, 14, 15,
	0,
	16, 17, 18, 19, 20,
	0, 0, 0, 0, 0, 0, 0, 0,
	21, 22, 23
};

static elementsN(mapToIndex);

static const Input::ButtonCode dirs[] =
{ Input::Down, Input::Left, Input::Right, Input::Up };

static const int dirFlags[] =
{
	1 << Input::Down,
	1 << Input::Left,
	1 << Input::Right,
	1 << Input::Up
};

/* Dir4 is always zero on these combinations */
static const int deadDirFlags[] =
{
	dirFlags[0] | dirFlags[3],
	dirFlags[1] | dirFlags[2]
};

static const Input::ButtonCode otherDirs[4][3] =
{
	{ Input::Left, Input::Right, Input::Up    }, /* Down  */
	{ Input::Down, Input::Up,    Input::Right }, /* Left  */
	{ Input::Down, Input::Up,    Input::Left  }, /* Right */
	{ Input::Left, Input::Right, Input::Up    }  /* Up    */
};

struct InputPrivate
{
	std::vector<KbBinding> kbStatBindings;
	std::vector<KbBinding> kbBindings;
	std::vector<JsAxisBinding> jsABindings;
	std::vector<JsHatBinding> jsHBindings;
	std::vector<JsButtonBinding> jsBBindings;
	std::vector<MsBinding> msBindings;

	/* Collective binding array */
	std::vector<Binding*> bindings;

	ButtonState stateArray[BUTTON_CODE_COUNT*2];

	ButtonState *states;
	ButtonState *statesOld;
    
    // Raw keystates
    uint8_t rawStateArray[SDL_NUM_SCANCODES*2];
    
    uint8_t *rawStates;
    uint8_t *rawStatesOld;

	Input::ButtonCode repeating;
    int rawRepeating;
	unsigned int repeatCount;
    unsigned int rawRepeatCount;

	struct
	{
		int active;
		Input::ButtonCode previous;
	} dir4Data;

	struct
	{
		int active;
	} dir8Data;


	InputPrivate(const RGSSThreadData &rtData)
	{
		initStaticKbBindings();
		initMsBindings();

		/* Main thread should have these posted by now */
		checkBindingChange(rtData);

		states    = stateArray;
		statesOld = stateArray + BUTTON_CODE_COUNT;
        
        rawStates = rawStateArray;
        rawStatesOld = rawStateArray + SDL_NUM_SCANCODES;

		/* Clear buffers */
		clearBuffer();
		swapBuffers();
		clearBuffer();

		repeating = Input::None;
		repeatCount = 0;

		dir4Data.active = 0;
		dir4Data.previous = Input::None;

		dir8Data.active = 0;
	}

	inline ButtonState &getStateCheck(int code)
	{
		int index;

		if (code < 0 || (size_t) code > mapToIndexN-1)
			index = 0;
		else
			index = mapToIndex[code];

		return states[index];
	}

	inline ButtonState &getState(Input::ButtonCode code)
	{
		return states[mapToIndex[code]];
	}

	inline ButtonState &getOldState(Input::ButtonCode code)
	{
		return statesOld[mapToIndex[code]];
	}
    
    inline ButtonState getStateRaw(int code)
    {
        ButtonState b;
        int scancode = -1;
        
        switch (code)
        {
                case 0x10:
                return getState(Input::Shift);
                break;
                
                case 0x11:
                return getState(Input::Ctrl);
                break;
                
                case 0x12:
                return getState(Input::Alt);
                break;
                
                case 0x1:
                return getState(Input::MouseLeft);
                break;
                
                case 0x2:
                return getState(Input::MouseRight);
                break;
                
                case 0x4:
                return getState(Input::MouseMiddle);
                break;
                
                default:
                ButtonState b;
                try
                {
                    scancode = vKeyToScancode[code];
                }
                catch (...) {}
                if (scancode == -1) return b;
                
                b.pressed = rawStates[scancode];
                b.triggered = (rawStates[scancode] && !rawStatesOld[scancode]);
                
                bool repeated = false;
                if (scancode == rawRepeating)
                {
                    if (rgssVer >= 2)
                        repeated = rawRepeatCount >= 23 && ((rawRepeatCount+1) % 6) == 0;
                    else
                        repeated = rawRepeatCount >= 15 && ((rawRepeatCount+1) % 4) == 0;
                }
                b.repeated = repeated;
                
                return b;
                break;
        }
    }

	void swapBuffers()
	{
		ButtonState *tmp = states;
		states = statesOld;
		statesOld = tmp;
        
        uint8_t *tmpr = rawStates;
        rawStates = rawStatesOld;
        rawStatesOld = tmpr;
	}

	void clearBuffer()
	{
		const size_t size = sizeof(ButtonState) * BUTTON_CODE_COUNT;
		memset(states, 0, size);
        
        memset(rawStates, 0, SDL_NUM_SCANCODES);
	}

	void checkBindingChange(const RGSSThreadData &rtData)
	{
		BDescVec d;

		if (!rtData.bindingUpdateMsg.poll(d))
			return;

		applyBindingDesc(d);
	}

	template<class B>
	void appendBindings(std::vector<B> &bind)
	{
		for (size_t i = 0; i < bind.size(); ++i)
			bindings.push_back(&bind[i]);
	}

	void applyBindingDesc(const BDescVec &d)
	{
		kbBindings.clear();
		jsABindings.clear();
		jsHBindings.clear();
		jsBBindings.clear();

		for (size_t i = 0; i < d.size(); ++i)
		{
			const BindingDesc &desc = d[i];
			const SourceDesc &src = desc.src;

			if (desc.target == Input::None)
				continue;

			switch (desc.src.type)
			{
			case Invalid :
				break;
			case Key :
			{
				KbBinding bind;
				bind.source = src.d.scan;
				bind.target = desc.target;
				kbBindings.push_back(bind);

				break;
			}
			case JAxis :
			{
				JsAxisBinding bind;
				bind.source = src.d.ja.axis;
				bind.dir = src.d.ja.dir;
				bind.target = desc.target;
				jsABindings.push_back(bind);

				break;
			}
			case JHat :
			{
				JsHatBinding bind;
				bind.source = src.d.jh.hat;
				bind.pos = src.d.jh.pos;
				bind.target = desc.target;
				jsHBindings.push_back(bind);

				break;
			}
			case JButton :
			{
				JsButtonBinding bind;
				bind.source = src.d.jb;
				bind.target = desc.target;
				jsBBindings.push_back(bind);

				break;
			}
			default :
				assert(!"unreachable");
			}
		}

		bindings.clear();

		appendBindings(kbStatBindings);
		appendBindings(msBindings);

		appendBindings(kbBindings);
		appendBindings(jsABindings);
		appendBindings(jsHBindings);
		appendBindings(jsBBindings);
	}

	void initStaticKbBindings()
	{
		kbStatBindings.clear();

		for (size_t i = 0; i < staticKbBindingsN; ++i)
			kbStatBindings.push_back(KbBinding(staticKbBindings[i]));
	}

	void initMsBindings()
	{
		msBindings.resize(3);

		size_t i = 0;
		msBindings[i++] = MsBinding(SDL_BUTTON_LEFT,   Input::MouseLeft);
		msBindings[i++] = MsBinding(SDL_BUTTON_MIDDLE, Input::MouseMiddle);
		msBindings[i++] = MsBinding(SDL_BUTTON_RIGHT,  Input::MouseRight);
	}

	void pollBindings(Input::ButtonCode &repeatCand)
	{
		for (size_t i = 0; i < bindings.size(); ++i)
			pollBindingPriv(*bindings[i], repeatCand);

		updateDir4();
		updateDir8();
	}

	void pollBindingPriv(const Binding &b,
	                     Input::ButtonCode &repeatCand)
	{	
		if (!b.sourceActive())
			return;

		if (b.target == Input::None)
			return;

		ButtonState &state = getState(b.target);
		ButtonState &oldState = getOldState(b.target);

		state.pressed = true;

		/* Must have been released before to trigger */
		if (!oldState.pressed)
			state.triggered = true;

		/* Unbound keys don't create/break repeat */
		if (repeatCand != Input::None)
			return;

		if (repeating != b.target &&
			!oldState.pressed)
		{
			if (b.sourceRepeatable())
				repeatCand = b.target;
			else
				/* Unrepeatable keys still break current repeat */
				repeating = Input::None;
		}
	}
    
    void updateRaw()
    {
        memcpy(rawStates, shState->eThread().keyStates, SDL_NUM_SCANCODES);
        
        for (int i = 0; i < SDL_NUM_SCANCODES; i++)
        {
            if (rawStates[i] && rawStatesOld[i])
            {
                if (rawRepeating == i)
                {
                    rawRepeatCount++;
                }
                else
                {
                    rawRepeatCount = 0;
                    rawRepeating = i;
                }
                
                break;
            }
        }
    }

	void updateDir4()
	{
		int dirFlag = 0;

		for (size_t i = 0; i < 4; ++i)
			dirFlag |= (getState(dirs[i]).pressed ? dirFlags[i] : 0);

		if (dirFlag == deadDirFlags[0] || dirFlag == deadDirFlags[1])
		{
			dir4Data.active = Input::None;
			return;
		}

		if (dir4Data.previous != Input::None)
		{
			/* Check if prev still pressed */
			if (getState(dir4Data.previous).pressed)
			{
				for (size_t i = 0; i < 3; ++i)
				{
					Input::ButtonCode other =
							otherDirs[(dir4Data.previous/2)-1][i];

					if (!getState(other).pressed)
						continue;

					dir4Data.active = other;
					return;
				}
			}
		}

		for (size_t i = 0; i < 4; ++i)
		{
			if (!getState(dirs[i]).pressed)
				continue;

			dir4Data.active = dirs[i];
			dir4Data.previous = dirs[i];
			return;
		}

		dir4Data.active   = Input::None;
		dir4Data.previous = Input::None;
	}

	void updateDir8()
	{
		static const int combos[4][4] =
		{
			{ 2, 1, 3, 0 },
			{ 1, 4, 0, 7 },
			{ 3, 0, 6, 9 },
			{ 0, 7, 9, 8 }
		};

		dir8Data.active = 0;

		for (size_t i = 0; i < 4; ++i)
		{
			Input::ButtonCode one = dirs[i];

			if (!getState(one).pressed)
				continue;

			for (int j = 0; j < 3; ++j)
			{
				Input::ButtonCode other = otherDirs[i][j];

				if (!getState(other).pressed)
					continue;

				dir8Data.active = combos[(one/2)-1][(other/2)-1];
				return;
			}

			dir8Data.active = one;
			return;
		}
	}
};


Input::Input(const RGSSThreadData &rtData)
{
	p = new InputPrivate(rtData);
}

void Input::update()
{
	shState->checkShutdown();
	p->checkBindingChange(shState->rtData());

	p->swapBuffers();
	p->clearBuffer();

	ButtonCode repeatCand = None;

	/* Poll all bindings */
	p->pollBindings(repeatCand);
    
    // Get raw keystates
    p->updateRaw();

	/* Check for new repeating key */
	if (repeatCand != None && repeatCand != p->repeating)
	{
		p->repeating = repeatCand;
		p->repeatCount = 0;
		p->getState(repeatCand).repeated = true;

		return;
	}

	/* Check if repeating key is still pressed */
	if (p->getState(p->repeating).pressed)
	{
		p->repeatCount++;

		bool repeated;
		if (rgssVer >= 2)
			repeated = p->repeatCount >= 23 && ((p->repeatCount+1) % 6) == 0;
		else
			repeated = p->repeatCount >= 15 && ((p->repeatCount+1) % 4) == 0;

		p->getState(p->repeating).repeated |= repeated;

		return;
	}

	p->repeating = None;
}

bool Input::isPressed(int button)
{
	return p->getStateCheck(button).pressed;
}

bool Input::isTriggered(int button)
{
	return p->getStateCheck(button).triggered;
}

bool Input::isRepeated(int button)
{
	return p->getStateCheck(button).repeated;
}

bool Input::isPressedEx(int vKey)
{
    return p->getStateRaw(vKey).pressed;
}

bool Input::isTriggeredEx(int vKey)
{
    return p->getStateRaw(vKey).triggered;
}

bool Input::isRepeatedEx(int vKey)
{
    return p->getStateRaw(vKey).repeated;
}

int Input::dir4Value()
{
	return p->dir4Data.active;
}

int Input::dir8Value()
{
	return p->dir8Data.active;
}

int Input::mouseX()
{
	RGSSThreadData &rtData = shState->rtData();
	
#ifdef __ANDROID__ 
	if (EventThread::touchState.ignoreMouse == false)
		for (size_t i = 0; i < MAX_FINGERS; ++i)
			{
				EventThread::FingerState &f = EventThread::touchState.fingers[i];
				if (!f.down) continue;
				return (f.x - rtData.screenOffset.x) * rtData.sizeResoRatio.x;
			}
#endif

	return (EventThread::mouseState.x - rtData.screenOffset.x) * rtData.sizeResoRatio.x;
}

int Input::mouseY()
{
	RGSSThreadData &rtData = shState->rtData();

#ifdef __ANDROID__ 
	if (EventThread::touchState.ignoreMouse == false)
		for (size_t i = 0; i < MAX_FINGERS; ++i)
			{
				EventThread::FingerState &f = EventThread::touchState.fingers[i];
				if (!f.down) continue;
				return (f.y - rtData.screenOffset.y) * rtData.sizeResoRatio.y;
			}
#endif
	
	return (EventThread::mouseState.y - rtData.screenOffset.y) * rtData.sizeResoRatio.y;
}

#define ks(sc) shState->eThread().keyStates[SDL_SCANCODE_##sc]
int Input::asyncKeyState(int key)
{
    int result;
    switch(key){
        case 0x10: // Any Shift
            result = (ks(LSHIFT) || ks(RSHIFT)) ? 0x8000 : 0;
            break;

        case 0x11: // Any Ctrl
            result = (ks(LCTRL) || ks(RCTRL)) ? 0x8000 : 0;
            break;

        case 0x12: // Any Alt
            result = (ks(LALT) || ks(RALT)) ? 0x8000 : 0;
            break;

        case 0x1: // Mouse button 1
            result = (SDL_GetMouseState(0, 0) & SDL_BUTTON(1)) ? 0x8000 : 0;
            break;

        case 0x2: // Mouse button 2
            result = (SDL_GetMouseState(0, 0) & SDL_BUTTON(3)) ? 0x8000 : 0;
            break;

        case 0x4: // Middle mouse
            result = (SDL_GetMouseState(0, 0) & SDL_BUTTON(2)) ? 0x8000 : 0;
            break;

        default:
            try {
                // Use EventThread instead of Input because
                // Input.update typically gets overridden
                result = shState->eThread().keyStates[vKeyToScancode[key]] << 15;
            } catch (...) {
                result = 0;
            }
        break;
    }
    return result;
}
#undef ks

Input::~Input()
{
	delete p;
}
