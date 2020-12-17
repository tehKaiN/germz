/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _GERMZ_STEER_H_
#define _GERMZ_STEER_H_

#include <ace/managers/joy.h> // for steerInitJoy() param
#include "direction.h"
#include "ai.h"

struct _tSteer;

typedef void (*tCbSteerProcess)(struct _tSteer *pSteer);

typedef enum _tSteerDirState {
	STEER_DIR_STATE_INACTIVE,
	STEER_DIR_STATE_USED,
	STEER_DIR_STATE_ACTIVE,
} tSteerDirState;

typedef enum _tKeymap {
	KEYMAP_WSAD,
	KEYMAP_ARROWS
} tKeymap;

typedef struct _tSteer {
	tCbSteerProcess cbProcess;
	tSteerDirState pDirectionStates[DIRECTION_COUNT];
	union {
		UBYTE ubJoy; ///< for joy steer
		tKeymap eKeymap; ///< for keyboard steer
		tAi sAi;
	};
} tSteer;

tSteer steerInitJoy(UBYTE ubJoy);

tSteer steerInitKey(tKeymap eKeymap);

tSteer steerInitAi(UBYTE ubPlayerIdx);

void steerProcess(tSteer *pSteer);

tSteer steerInitIdle(void);

UBYTE steerIsPlayer(const tSteer *pSteer);

UBYTE steerIsArrows(const tSteer *pSteer);

UBYTE steerDirCheck(const tSteer *pSteer, tDirection eDir);

UBYTE steerDirUse(tSteer *pSteer, tDirection eDir);

tDirection steerGetPressedDir(const tSteer *pSteer);

void steerResetAi(tSteer *pSteer);

#endif // _GERMZ_STEER_H_
