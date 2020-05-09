/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "player.h"
#include "game.h"

//---------------------------------------------------------------------- DEFINES

#define PLAYER_COUNT_MAX 4

//------------------------------------------------------------------------ TYPES

//----------------------------------------------------------------- PRIVATE VARS

tPlayer s_pPlayers[PLAYER_COUNT_MAX];

//------------------------------------------------------------------ PRIVATE FNS

static void playerSetCursorPos(tPlayer *pPlayer, tNode *pNode) {
		pPlayer->pNodeCursor = pNode;
		pPlayer->pBobCursor->sPos.uwX = pNode->ubTileX * MAP_TILE_SIZE;
		pPlayer->pBobCursor->sPos.uwY = pNode->ubTileY * MAP_TILE_SIZE;
}

static void playerTryMoveSelectionFromInDir(
	tPlayer *pPlayer, tNode *pNode, tDir eDir
) {
	if(eDir != DIR_COUNT && pNode->pNeighbors[eDir]) {
		playerSetCursorPos(pPlayer, pNode->pNeighbors[eDir]);
		pPlayer->eLastDir = eDir;
	}
}

static void playerSpawnPlep(tPlayer *pPlayer) {
	// TODO: store last index, add starting from it
	WORD wPlepCharges = pPlayer->pNodePlepSrc->wCharges / 2;
	if(
		!wPlepCharges || pPlayer->eLastDir >= DIR_FIRE ||
		!pPlayer->pNodePlepSrc->pNeighbors[pPlayer->eLastDir]
	) {
		return;
	}
	for(UBYTE i = 0; i < PLEPS_PER_PLAYER; ++i) {
		tPlep *pPlep = &pPlayer->pPleps[i];
		if(!pPlep->isActive && pPlayer->pNodePlepSrc->pPlayer == pPlayer) {
			plepSpawn(pPlep, wPlepCharges, pPlayer->eLastDir);
			pPlayer->eLastDir = DIR_COUNT;
			pPlayer->pNodePlepSrc->wCharges -= wPlepCharges;
			logWrite(
				"Spawned plep %hhu on player %hhu: blob %hhu,%hhu -> %hhu,%hhu\n",
				i, playerToIdx(pPlayer),
				pPlayer->pNodePlepSrc->ubTileX, pPlayer->pNodePlepSrc->ubTileY,
				pPlep->pDestination->ubTileX, pPlep->pDestination->ubTileY
			);
			break;
		}
	}
}

//------------------------------------------------------------------- PUBLIC FNS

void playerCreate(void) {
	logBlockBegin("playerCreate()");
	plepCreate();

	for(UBYTE ubIdx = 0; ubIdx < 4; ++ubIdx) {
		tPlayer *pPlayer = &s_pPlayers[ubIdx];
		for(UBYTE ubPlep = 0; ubPlep < PLEPS_PER_PLAYER; ++ubPlep) {
			plepInitBob(&pPlayer->pPleps[ubPlep]);
		}
	}
	logBlockEnd("playerCreate()");
}

void playerDestroy(void) {
	plepDestroy();
}

void playerReset(UBYTE ubIdx, tNode *pStartNode) {
	tPlayer *pPlayer = &s_pPlayers[ubIdx];
	pPlayer->pBobCursor = gameGetCursorBob(ubIdx);
	for(UBYTE ubPlep = 0; ubPlep < PLEPS_PER_PLAYER; ++ubPlep) {
		plepReset(&pPlayer->pPleps[ubPlep], pPlayer);
	}
	logWrite(
		"Player %hhu start pos: %hhu,%hhu\n",
		ubIdx, pStartNode->ubTileX, pStartNode->ubTileY
	);
	playerSetCursorPos(pPlayer, pStartNode);
	pPlayer->isSelectingDestination = 0;
	pPlayer->pSteer = gameGetSteerForPlayer(ubIdx);
	pPlayer->isDead = 0;
	pPlayer->bNodeCount = 0;
	pPlayer->eLastDir = DIR_COUNT;
}

tTile playerToTile(const tPlayer *pPlayer) {
	tTile eTile = TILE_BLOB_NEUTRAL + playerToIdx(pPlayer);
	return eTile;
}

tPlayer *playerFromTile(tTile eTile) {
	tPlayer *pPlayer = 0;
	if(eTile >= TILE_BLOB_P1) {
		pPlayer = &s_pPlayers[eTile - TILE_BLOB_P1];
	}
	return pPlayer;
}

UBYTE playerToIdx(const tPlayer *pPlayer) {
	UBYTE ubPlayerIdx = 0;
	if(pPlayer) {
		ubPlayerIdx = 1 + (UBYTE)(pPlayer - &s_pPlayers[0]);
	}
	return ubPlayerIdx;
}

tPlayer *playerFromIdx(UBYTE ubIdx) {
	tPlayer *pPlayer = 0;
	if(ubIdx > 0) {
		pPlayer = &s_pPlayers[ubIdx - 1];
	}
	return pPlayer;
}

UBYTE playerProcess(void) {
	UBYTE ubAliveCount = 0;
	for(UBYTE i = 0; i < PLAYER_COUNT_MAX; ++i) {
		tPlayer *pPlayer = &s_pPlayers[i];
		if(pPlayer->isDead) {
			continue;
		}
		++ubAliveCount;
		tDir eDir = steerProcess(pPlayer->pSteer);
		if(pPlayer->isSelectingDestination) {
			if(eDir == DIR_FIRE) {
				playerSpawnPlep(pPlayer);
				playerSetCursorPos(pPlayer, pPlayer->pNodePlepSrc);
				pPlayer->isSelectingDestination = 0;
			}
			else {
				playerTryMoveSelectionFromInDir(pPlayer, pPlayer->pNodePlepSrc, eDir);
			}
		}
		else {
			if(eDir == DIR_FIRE) {
				if(pPlayer->pNodeCursor->pPlayer == pPlayer) {
					pPlayer->pNodePlepSrc = pPlayer->pNodeCursor;
					pPlayer->isSelectingDestination = 1;
				}
			}
			else {
				playerTryMoveSelectionFromInDir(pPlayer, pPlayer->pNodeCursor, eDir);
			}
		}

		for(UBYTE i = 0; i < PLEPS_PER_PLAYER; ++i) {
			if(pPlayer->pPleps[i].isActive) {
				plepProcess(&pPlayer->pPleps[i]);
			}
		}
		bobNewPush(pPlayer->pBobCursor);
	}
	return ubAliveCount;
}

void playerUpdateDead(tPlayer *pPlayer) {
	// Check if player has some blobs or is dead already
	logWrite("playerUpdateDead: %p has %hhd blobs\n", pPlayer, pPlayer->bNodeCount);
	if(pPlayer->isDead || pPlayer->bNodeCount > 0) {
		return;
	}

	// Check if player has any plep
	for(UBYTE i = 0; i < PLEPS_PER_PLAYER; ++i) {
		if(pPlayer->pPleps[i].isActive) {
			logWrite("has active pleps!\n");
			return;
		}
	}

	// Nothing more to check - player is dead
	logWrite("yep he ded\n");
	pPlayer->isDead = 1;
}

void playerAllDead(void) {
	for(UBYTE i = 0; i < PLAYER_COUNT_MAX; ++i) {
		s_pPlayers[i].isDead = 1;
	}
}
