/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <ace/generic/main.h>
#include <ace/managers/key.h>
#include <ace/managers/joy.h>
#include "menu.h"

void genericCreate(void) {
	keyCreate();
	joyOpen();
	gamePushState(menuGsCreate, menuGsLoop, menuGsDestroy);
}

void genericProcess(void) {
	keyProcess();
	joyProcess();
	gameProcess();
}

void genericDestroy(void) {
	keyDestroy();
	joyClose();
}
