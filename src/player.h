/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _GERMZ_PLAYER_H_
#define _GERMZ_PLAYER_H_

#include "bob_new.h"
#include "map.h"
#include "plep.h"
#include "steer.h"

#define PLEPS_PER_PLAYER 3

typedef struct _tPlayer {
	BYTE bNodeCount;
	UBYTE isSelectingDestination;
	UBYTE isDead;
	tBobNew *pBobCursor;
	struct _tSteer *pSteer;
	struct _tNode *pNodeCursor, *pNodePlepSrc;
	struct _tPlep pPleps[PLEPS_PER_PLAYER];
	tDir eLastDir;
} tPlayer;

void playerCreate(void);

void playerDestroy(void);

/**
 * @brief
 *
 * @param ubIdx 0: P1
 * @param pStartNode
 * @param sSteer
 */
void playerReset(UBYTE ubIdx, struct _tNode *pStartNode);

/**
 * @brief
 *
 * @return Number of players alive.
 */
UBYTE playerProcess(void);

enum _tTile playerToTile(const tPlayer *pPlayer);

tPlayer *playerFromTile(enum _tTile eTile);

/**
 * @brief
 *
 * @param pPlayer
 * @return 0 on neutral/null, 1-4 for P1-P4
 */
UBYTE playerToIdx(const tPlayer *pPlayer);

tPlayer *playerFromIdx(UBYTE ubIdx);

void playerUpdateDead(tPlayer *pPlayer);

void playerAllDead(void);

#endif // _GERMZ_PLAYER_H_
