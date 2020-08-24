/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dialog_load.h"
#include <gui/dialog.h>
#include <gui/list_ctl.h>
#include <gui/button.h>
#include <ace/utils/dir.h>
#include <ace/managers/game.h>
#include <ace/managers/key.h>
#include "game_editor.h"
#include "dialog_save.h"
#include "game.h"
#include "game_assets.h"
#include "germz.h"

#define MAP_FILENAME_MAX 100

static tListCtl *s_pCtrl;
static tMapData *s_pPreview;
static const char *s_szFilePrev;
static tBitMap *s_pBmDialog;
static ULONG s_ullChangeTimer;
static UBYTE s_isMapInfoRefreshed;

static void clearMapInfo(tBitMap *pBmDialog) {
	const UWORD uwOffsX = s_pCtrl->sRect.uwWidth + 3;
	UWORD uwOffsY = 3;
	const UBYTE ubRowWidth = 100;
	const UBYTE ubRowHeight = g_pFont->uwHeight + 2;

	blitRect(pBmDialog, uwOffsX, uwOffsY, ubRowWidth, ubRowHeight, 0);
	fontFillTextBitMap(g_pFont, g_pTextBitmap, "Loading map...");
	fontDrawTextBitMap(pBmDialog, g_pTextBitmap, uwOffsX, uwOffsY, 17, FONT_COOKIE);

	uwOffsY += ubRowHeight;
	blitRect(pBmDialog, uwOffsX, uwOffsY, ubRowWidth, ubRowHeight, 0);

	s_isMapInfoRefreshed = 0;
}

static void updateMapInfo(tBitMap *pBmDialog) {
	const char *szFile = listCtlGetSelection(s_pCtrl);
	if(szFile == s_szFilePrev) {
		return;
	}

	char szPath[MAP_FILENAME_MAX];
	sprintf(szPath, "data/maps/%s.json", szFile);
	mapDataInitFromFile(s_pPreview, szPath);

	UWORD uwOffsX = s_pCtrl->sRect.uwWidth + 3;
	UWORD uwOffsY = 3;
	const UBYTE ubRowWidth = 100;
	const UBYTE ubRowHeight = g_pFont->uwHeight + 2;

	char szLine[10 + MAX(MAP_NAME_MAX, MAP_AUTHOR_MAX)];
	blitRect(pBmDialog, uwOffsX, uwOffsY, ubRowWidth, ubRowHeight, 0);
	sprintf(szLine, "Title: %s", s_pPreview->szName);
	fontFillTextBitMap(g_pFont, g_pTextBitmap, szLine);
	fontDrawTextBitMap(pBmDialog, g_pTextBitmap, uwOffsX, uwOffsY, 17, FONT_COOKIE);

	uwOffsY += ubRowHeight;
	blitRect(pBmDialog, uwOffsX, uwOffsY, ubRowWidth, ubRowHeight, 0);
	sprintf(szLine, "Author: %s", s_pPreview->szAuthor);
	fontFillTextBitMap(g_pFont, g_pTextBitmap, szLine);
	fontDrawTextBitMap(pBmDialog, g_pTextBitmap, uwOffsX, uwOffsY, 17, FONT_COOKIE);
	uwOffsY += ubRowHeight;

	// Draw map
	static const UBYTE pTileColors[TILE_COUNT] = {
		[TILE_BLOB_P1] = 10,
		[TILE_BLOB_P2] = 14,
		[TILE_BLOB_P3] = 18,
		[TILE_BLOB_P4] = 22,
		[TILE_BLOB_NEUTRAL] = 6,
		[TILE_BLANK] = 7,
		[TILE_EDITOR_BLANK] = 7,
		[TILE_PATH_H1] = 26,
		[TILE_PATH_H2] = 26,
		[TILE_PATH_H3] = 26,
		[TILE_PATH_H4] = 26,
		[TILE_PATH_V1] = 26,
		[TILE_PATH_V2] = 26,
		[TILE_PATH_V3] = 26,
		[TILE_PATH_V4] = 26,
		[TILE_PATH_X1] = 26,
		[TILE_PATH_X2] = 26,
		[TILE_PATH_X3] = 26,
		[TILE_PATH_X4] = 26,
	};

	uwOffsX += (((256 - uwOffsX) - 66) / 2);
	blitRect(pBmDialog, uwOffsX - 1, uwOffsY - 1, 64 + 2,      1, 16);
	blitRect(pBmDialog, uwOffsX - 1, uwOffsY - 1,      1, 64 + 2, 16);
	blitRect(pBmDialog, uwOffsX     , uwOffsY + 64, 64 + 1,      1, 19);
	blitRect(pBmDialog, uwOffsX + 64, uwOffsY     ,      1, 64 + 1, 19);
	for(UBYTE ubY = 0; ubY < MAP_SIZE; ++ubY) {
		for(UBYTE ubX = 0; ubX < MAP_SIZE; ++ubX) {
			UBYTE ubColor = pTileColors[s_pPreview->pTiles[ubX][ubY]];
			blitRect(pBmDialog, uwOffsX + 4 * ubX, uwOffsY + 4 * ubY, 4, 4, ubColor);
		}
	}

	s_isMapInfoRefreshed = 1;
	s_szFilePrev = szFile;
}

static void dialogLoadGsCreate(void) {
	UWORD uwDlgWidth = 256;
	UWORD uwDlgHeight = 128;
	s_pBmDialog = dialogCreate(
		uwDlgWidth, uwDlgHeight, gameGetBackBuffer(), gameGetFrontBuffer()
	);
	s_pPreview = memAllocFast(sizeof(*s_pPreview));
	s_szFilePrev = 0;
	s_isMapInfoRefreshed = 0;

	// Initial draw
	UBYTE ubPad = 1;
	UWORD uwWidth = uwDlgWidth / 2;
	UWORD uwHeight = uwDlgHeight - 2;

	buttonListCreate(5, s_pBmDialog, g_pFont, g_pTextBitmap);
	s_pCtrl = listCtlCreate(
		s_pBmDialog, ubPad, ubPad, uwWidth, uwHeight, g_pFont, 10, g_pTextBitmap, 0
	);

	tDir *pDir = dirOpen("data/maps");
	if(!pDir) {
		dirCreate("data/maps");
		pDir = dirOpen("data/maps");
	}
	if(!pDir) {
		// TODO: something better
		logWrite("Can't open or create maps dir!\n");
		statePop(g_pStateMachineGame);
		return;
	}

	// Count relevant files
	char szFileName[MAP_FILENAME_MAX];
	while(dirRead(pDir, szFileName, MAP_FILENAME_MAX)) {
		UWORD uwLen = strlen(szFileName);
		if(uwLen < 20 && !strcmp(&szFileName[uwLen - 5], ".json")) {
			// Trim extension and add to list
			szFileName[strlen(szFileName) - strlen(".json")] = '\0';
			listCtlAddEntry(s_pCtrl, szFileName);
		}
	}
	dirClose(pDir);
	listCtlSortEntries(s_pCtrl);

	updateMapInfo(s_pBmDialog);
	s_ullChangeTimer = timerGet();

	const UWORD uwBtnWidth = 50;
	const UWORD uwBtnHeight = 20;
	tButton *pButtonLoad = buttonAdd(
		uwWidth + (uwDlgWidth - uwWidth - uwBtnWidth) / 2,
		uwDlgHeight - uwBtnHeight - 10,
		uwBtnWidth, uwBtnHeight, "Load", 0, 0
	);
	listCtlDraw(s_pCtrl);
	buttonSelect(pButtonLoad);
	buttonDrawAll();
}

static void dialogLoadGsLoop(void) {
	if(!gamePreprocess()) {
		return;
	}

	UBYTE isMapSelected = 0;
	tDirection eDir = gameEditorGetSteerDir();
	if(eDir == DIRECTION_UP) {
		listCtlSelectPrev(s_pCtrl);
		clearMapInfo(s_pBmDialog);
		s_ullChangeTimer = timerGet();
	}
	else if(eDir == DIRECTION_DOWN) {
		listCtlSelectNext(s_pCtrl);
		clearMapInfo(s_pBmDialog);
		s_ullChangeTimer = timerGet();
	}
	else if(eDir == DIRECTION_FIRE || keyUse(KEY_RETURN) || keyUse(KEY_NUMENTER)) {
		// No clicking the button - code is shorter that way
		if(!s_isMapInfoRefreshed) {
			updateMapInfo(s_pBmDialog);
		}
		memcpy(&g_sMapData, s_pPreview, sizeof(g_sMapData));
		dialogSaveSetSaveName(listCtlGetSelection(s_pCtrl));
		isMapSelected = 1;
	}

	if(timerGetDelta(s_ullChangeTimer, timerGet()) >= 25) {
		updateMapInfo(s_pBmDialog);
		s_ullChangeTimer = timerGet();
	}

	// Copy dialog bitmap to screen
	dialogProcess(gameGetBackBuffer());

	gamePostprocess();

	if(isMapSelected || keyUse(KEY_ESCAPE)) {
		statePop(g_pStateMachineGame);
	}
}

static void dialogLoadGsDestroy(void) {
	memFree(s_pPreview, sizeof(*s_pPreview));
	buttonListDestroy();
	listCtlDestroy(s_pCtrl);
	dialogDestroy();
}


void dialogLoadShow(void) {
	statePush(g_pStateMachineGame, &g_sStateDialogLoad);
}

tState g_sStateDialogLoad = {
	.cbCreate = dialogLoadGsCreate, .cbLoop = dialogLoadGsLoop,
	.cbDestroy = dialogLoadGsDestroy
};
