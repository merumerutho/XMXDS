/*
 * channelMatrix.h
 *
 *  Created on: 20 gen 2023
 *      Author: merut
 */

#ifndef ARM9_SOURCE_CHANNELMATRIX_H_
#define ARM9_SOURCE_CHANNELMATRIX_H_

#include <nds.h>

/* Full redraw of the channel matrix (clears bottom screen). */
void drawChannelMatrix(void);

/* Incremental update of a single channel cell (mute + solo rows). */
void drawChannelCell(u8 idx);

/* Handle a touch on the channel screen.
   Top half of a cell = mute toggle; bottom half = solo toggle.
   Unused channels (idx >= module channel count) are ignored. */
void handleChannelTouch(touchPosition *touchPos);

#endif /* ARM9_SOURCE_CHANNELMATRIX_H_ */
