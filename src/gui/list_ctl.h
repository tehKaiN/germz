/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _GUI_LIST_CTL_H_
#define _GUI_LIST_CTL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <ace/types.h>
#include <ace/utils/font.h>
#include <gui/background.h>

#define LISTCTL_ENTRY_INVALID 0xFFFF
#define LISTCTL_DRAWSTATE_OK 0
#define LISTCTL_DRAWSTATE_NEEDS_REDRAW 1

typedef void (*tCbListCtlOnSelect)(void);

typedef struct _tListCtl {
	tUwRect sRect;
	UBYTE ubDrawState;
	UBYTE ubEntryHeight;
	UWORD uwEntryCnt;
	UWORD uwEntryMaxCnt;
	UWORD uwEntrySel;
	UWORD uwEntryScrollPos;
	char **pEntries;
	tFont *pFont;
	const tGuiBackground *pBg;
	tBitMap *pBfr;
	tCbListCtlOnSelect cbOnSelect;
	tTextBitMap *pEntryTextBfr;
	UBYTE isTextBfrAlloc;
} tListCtl;

tListCtl *listCtlCreate(
	const tGuiBackground *pBg, tBitMap *pBfr, UWORD uwX, UWORD uwY,
	UWORD uwWidth, UWORD uwHeight, tFont *pFont, UWORD uwEntryMaxCnt,
	tTextBitMap *pTextBfr, tCbListCtlOnSelect cbOnSelect
);

void listCtlDestroy(tListCtl *pCtl);

UWORD listCtlAddEntry(tListCtl *pCtl, const char *szTxt);

void listCtlRemoveEntry(tListCtl *pCtl, UWORD uwIdx);

void listCtlDraw(tListCtl *pCtl);

void listCtlUndraw(tListCtl *pCtl);

UBYTE listCtlProcessClick(tListCtl *pCtl, UWORD uwMouseX, UWORD uwMouseY);

/**
 * @brief Selects previous entry on the list.
 *
 * @param pCtl
 * @return 1 if selection was changed, otherwise 0.
 */
UBYTE listCtlSelectPrev(tListCtl *pCtl);

/**
 * @brief Selects next entry on the list.
 *
 * @param pCtl
 * @return 1 if selection was changed, otherwise 0.
 */
UBYTE listCtlSelectNext(tListCtl *pCtl);

void listCtlSortEntries(tListCtl *pCtl);

const char *listCtlGetSelection(const tListCtl *pCtl);

void listCtlSetSelectionIdx(tListCtl *pCtl, UWORD uwIdx);

#ifdef __cplusplus
}
#endif

#endif // _GUI_LIST_CTL_H_
