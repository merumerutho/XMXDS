#include <nds.h>
#include <stdlib.h>  // for malloc()
#include <string.h>  // for memcpy(), memcmp()
#include <stddef.h>

#include <stdio.h>

#include "../../arm7/include/libxm7.h"

// MOD octave 0 difference
#define AMIGABASEOCTAVE 2
// AmigaPeriods for MOD "Octave ZERO"
u16 AmigaPeriods[12] = { 1712, 1616, 1525, 1440, 1357, 1281, 1209, 1141, 1077, 1017, 961, 907 };

u16 GetAmigaPeriod(u8 note)
{  // note from 0 to 95

    u16 period = AmigaPeriods[note % 12];
    int octave = (note / 12) - AMIGABASEOCTAVE;

    if (octave > 0)
        period >>= octave;
    else if (octave < 0) period <<= -octave;

    return (period);
}

u8 FindClosestNoteToAmigaPeriod(u16 period)
{  // note from 0 to 95
    u8 note = 0;
    u16 bottomperiod;
    u16 topperiod;

    // jump octaves
    topperiod = GetAmigaPeriod(note);
    while (topperiod >= (period * 2))
    {
        note += 12;
        topperiod = GetAmigaPeriod(note);
    }

    // jump notes
    bottomperiod = topperiod;
    while (topperiod > period)
    {
        bottomperiod = topperiod;
        note++;
        topperiod = GetAmigaPeriod(note);
    }

    // find closest
    if ((period - topperiod) <= (bottomperiod - period))
        return (note);
    else
        return (note - 1);
}

u16 SwapBytes(u16 in)
{
    return ((in & 0x00FF) << 8 | (in >> 8));
}

u8 IdentifyMOD(char c1, char c2, char c3, char c4)
{
    // 4 channels MOD
    if ((c1 == 'M') && (c2 == '.') && (c3 == 'K') && (c4 == '.'))       // M.K.
        return (4);

    if ((c1 == 'M') && (c2 == '!') && (c3 == 'K') && (c4 == '!'))       // M!K!
        return (4);

    if ((c1 == 'F') && (c2 == 'L') && (c3 == 'T') && (c4 == '4'))       // FLT4 [Startrekker]
        return (4);

    //if ((c1=='E') && (c2=='X') && (c3=='O') && (c4=='4'))     // EXO4 (should be FM)
    //  return (4);

    // 8 channels MOD    
    if ((c1 == 'O') && (c2 == 'C') && (c3 == 'T') && (c4 == 'A'))       // OCTA
        return (8);

    if ((c1 == 'C') && (c2 == 'D') && (c3 == '8') && (c4 == '1'))       // CD81 [Falcon]
        return (8);

    // FLT8 support (!!!)
    if ((c1 == 'F') && (c2 = 'L') && (c3 = 'T') && (c4 = '8'))           // FLT8 [Startrekker]
        return (8 | 0x80);

    // if ((c1=='E') && (c2='X') && (c3='O') && (c4='8'))        // EXO8 (same as FLT8) (should be FM)
    //   return (8 | 0x80);

    // xCHN (2-4-6-8 chns & 5-7-9 chns)
    if ((c2 == 'C') && (c3 == 'H') && (c4 == 'N'))
    {                                  // ?CHN
        if ((c1 == '2') || (c1 == '4') || (c1 == '6') || (c1 == '8') ||  // 2CHN - 4CHN(?) - 6CHN - 8CHN
            (c1 == '5') || (c1 == '7') || (c1 == '9'))                 // 5CHN - 7CHN - 9CHN [TakeTracker]
            return (c1 - '0');
    }

    // xxCH  (10-12-14-16-18-20-22-24-26-28-30-32 chns & 11-13-15 [TakeTracker])
    if ((c3 == 'C') && (c4 == 'H'))
    {
        if ((c1 >= '1') && (c1 <= '3') && (c2 >= '0') && (c2 <= '8'))
        {
            return ((c1 - '0') * 10 + (c2 - '0'));
        }
    }

    // TDZx (1-2-3 chns [TakeTracker])
    if ((c1 == 'T') && (c2 == 'D') && (c3 == 'Z'))					// TDZ?
    {
        if ((c4 == '1') || (c4 == '2') || (c4 == '3'))                                // TDZ1 - TDZ2 - TDZ3 [TakeTracker]
            return (c4 - '0');
    }

    return (0);
}

XM7_SingleNoteArray_Type* PrepareNewPattern(u16 len, u8 chn)
{
    // prepares a new EMPTY pattern with LEN lines and CNH channels

    XM7_SingleNoteArray_Type *ptr;
    u16 i, cnt;

    cnt = len * chn;
    ptr = (XM7_SingleNoteArray_Type*) malloc(sizeof(XM7_SingleNote_Type) * cnt);

    // check if memory has been allocated before using it    
    if (ptr != NULL)
    {
        for (i = 0; i < cnt; i++)
        {
            ptr->Noteblock[i].Note = 0x00;
            ptr->Noteblock[i].Instrument = 0x00;
            ptr->Noteblock[i].Volume = 0x00;
            ptr->Noteblock[i].EffectType = 0x00;
            ptr->Noteblock[i].EffectParam = 0x00;
        }
    }

    return (ptr);
}

XM7_Instrument_Type* PrepareNewInstrument(void)
{
    // prepares a new EMPTY instrument

    XM7_Instrument_Type *ptr = (XM7_Instrument_Type*) malloc(sizeof(XM7_Instrument_Type));

    // check if memory has been allocated before using it
    if (ptr != NULL)
    {

        ptr->NumberofSamples = 0;                     // 0 samples when empty

        int i;
        for (i = 0; i < 16; i++)
            ptr->Sample[i] = NULL;                    // all the pointer to the samples are NULL

        // set the name to empty    
        memset(ptr->Name, 0, 22);                  // 22 asciizero
    }

    return (ptr);
}

XM7_Sample_Type* PrepareNewSample(u32 len, u32 looplen, u8 flags)
{
    // prepares a new EMPTY sample

    XM7_SampleData_Type *data_ptr;
    XM7_Sample_Type *ptr;

    u32 malloclen = len;

    if ((flags & 0x03) == 0x02)
    {    // it's a ping-pong loop so malloclen should be calculated 'better'

        /*
         if ((flags & 0x10) == 0) {
         // 8 bit/sample
         malloclen += (looplen - 2);                 // -2 samples because of the 1st and last which shouldn't be duplicated
         }
         else
         {
         // 16 bit/sample
         malloclen += (looplen - 4);                 // -2 samples (-4 bytes) because of the 1st and last which shouldn't be duplicated
         }
         */
        malloclen += looplen;                   // adds the portion that gets reverted
    }

    ptr = (XM7_Sample_Type*) malloc(sizeof(XM7_Sample_Type));

    // check if memory has been allocated before using it
    if (ptr != NULL)
    {
        data_ptr = (XM7_SampleData_Type*) malloc(sizeof(u8) * malloclen);

        // check if SAMPLE memory has been allocated before using it
        if (data_ptr != NULL)
        {

            ptr->SampleData = data_ptr;
            ptr->Length = len;                      // yes, len, not malloclen.
                                                    //  this will be fixed later
            // set the name to empty    
            memset(ptr->Name, 0, 22);   // 22 asciizero
        }
        else
        {
            // SAMPLE memory not allocated, REMOVE the parent
            free(ptr);
            ptr = NULL;
        }
    }

    return (ptr);
}

u16 XM7_LoadXM(XM7_ModuleManager_Type *Module, XM7_XMModuleHeader_Type *XMModule)
{    // returns 0 if OK, an error otherwise

    // Clear screen
    consoleClear();
    iprintf("Loading...\n");

    // reset these values
    Module->NumberofPatterns = 0;
    Module->NumberofInstruments = 0;

    // check the ID text and the 0x1a
    if ((memcmp(XMModule->FixedText, "Extended Module: ", 17) != 0) || (XMModule->FixedChar != 0x1a))
    {
        Module->State = XM7_STATE_ERROR | XM7_ERR_NOT_A_VALID_MODULE;
        return (XM7_ERR_NOT_A_VALID_MODULE);
    }

    // check the version of the module
    if ((XMModule->Version != 0x103) && (XMModule->Version != 0x104))
    {
        Module->State = XM7_STATE_ERROR | XM7_ERR_UNKNOWN_MODULE_VERSION;
        return (XM7_ERR_UNKNOWN_MODULE_VERSION);
    }

    // check how may channels are in the module
    if (XMModule->NumberofChannels > LIBXM7_MAX_CHANNELS_PER_MODULE)
    {
        Module->State = XM7_STATE_ERROR | XM7_ERR_UNSUPPORTED_NUMBER_OF_CHANNELS;
        return (XM7_ERR_UNSUPPORTED_NUMBER_OF_CHANNELS);
    }

    // load all the needed info from the header
    Module->ModuleLength = XMModule->SongLength;
    Module->RestartPoint = XMModule->RestartPosition;
    Module->NumberofChannels = XMModule->NumberofChannels;
    Module->NumberofInstruments = XMModule->NumberofInstruments;
    Module->NumberofPatterns = XMModule->NumberofPatterns;
    Module->FreqTable = XMModule->XMModuleFlags;
    Module->DefaultTempo = XMModule->DefaultTempo;
    Module->DefaultBPM = XMModule->DefaultBPM;

    // Set current to defaults
    Module->CurrentBPM = Module->DefaultBPM;
    Module->CurrentTempo = Module->DefaultTempo;

    // By default un-mute all channels available
    for (u8 i = 0; i < Module->NumberofChannels; i++)
        Module->ChannelMute[i] = 0;
    // Mute those that aren't present, by default
    for (u8 i = Module->NumberofChannels; i < 16; i++)
        Module->ChannelMute[i] = 1;

    memcpy(Module->ModuleName, XMModule->XMModuleName, 20);  // char[20]
    memcpy(Module->TrackerName, XMModule->TrackerName, 20);  // char[20]
    memcpy(Module->PatternOrder, XMModule->PatternOrder, XMModule->HeaderSize - 20);    // u8[]

    // the MODULE header is finished!

    // BETA TEST
    // return (0);

    // now working on the patterns
    u16 CurrentPattern;
    XM7_XMPatternHeader_Type *XMPatternHeader = (XM7_XMPatternHeader_Type*) &(XMModule->PatternOrder[XMModule->HeaderSize - 20]);

    // BETA TEST
    // Module->NumberofPatterns=1;

    for (CurrentPattern = 0; CurrentPattern < (Module->NumberofPatterns); CurrentPattern++)
    {

        // check if the PATTERN header is ok
        if ((XMPatternHeader->HeaderLength != 9) || (XMPatternHeader->PackingType != 0))
        {
            Module->NumberofPatterns = CurrentPattern;
            Module->NumberofInstruments = 0;
            Module->State = XM7_STATE_ERROR | XM7_ERR_UNSUPPORTED_PATTERN_HEADER;
            return (XM7_ERR_UNSUPPORTED_PATTERN_HEADER);
        }

        // check if the PATTERN lenght is ok
        if ((XMPatternHeader->NumberofLinesinThisPattern < 1) || (XMPatternHeader->NumberofLinesinThisPattern > 256))
        {
            Module->NumberofPatterns = CurrentPattern;
            Module->NumberofInstruments = 0;
            Module->State = XM7_STATE_ERROR | XM7_ERR_UNSUPPORTED_PATTERN_HEADER;
            return (XM7_ERR_UNSUPPORTED_PATTERN_HEADER);
        }

        // pattern is ok! Get the length
        Module->PatternLength[CurrentPattern] = XMPatternHeader->NumberofLinesinThisPattern;

        // Prepare an empty pattern for the data
        Module->Pattern[CurrentPattern] = PrepareNewPattern(Module->PatternLength[CurrentPattern], Module->NumberofChannels);
        if (Module->Pattern[CurrentPattern] == NULL)
        {
            Module->NumberofPatterns = CurrentPattern;
            Module->NumberofInstruments = 0;
            Module->State = XM7_STATE_ERROR | XM7_ERR_NOT_ENOUGH_MEMORY;
            return (XM7_ERR_NOT_ENOUGH_MEMORY);
        }

        // decode the PackedData
        u16 i = 0;
        u16 wholenote = 0;
        u8 firstbyte;

        XM7_SingleNoteArray_Type *thispattern = Module->Pattern[CurrentPattern];

        while (i < (XMPatternHeader->PackedPatterndataLength))
        {
            firstbyte = XMPatternHeader->PatternData[i];

            if (firstbyte & 0x80)
            {
                // it's compressed: skip to the next byte
                i++;
            }
            else
            {
                // if "it's NOT compressed" then there are 5 bytes, simulate it's compressed with 5 bytes following
                firstbyte = 0x1F;
            }

            // if next is a NOTE:
            if (firstbyte & 0x01)
            {
                // read the note
                thispattern->Noteblock[wholenote].Note = XMPatternHeader->PatternData[i];
                i++;
            }

            // if next is a INSTRUMENT:
            if (firstbyte & 0x02)
            {
                // read the instrument
                thispattern->Noteblock[wholenote].Instrument = XMPatternHeader->PatternData[i];
                i++;
            }

            // if next is a VOLUME:
            if (firstbyte & 0x04)
            {
                // read the volume
                thispattern->Noteblock[wholenote].Volume = XMPatternHeader->PatternData[i];
                i++;
            }

            // if next is an EFFECT TYPE:
            if (firstbyte & 0x08)
            {
                // read the effect type
                thispattern->Noteblock[wholenote].EffectType = XMPatternHeader->PatternData[i];
                i++;
            }

            // if next is an EFFECT PARAM:
            if (firstbyte & 0x10)
            {
                // read the effect param
                thispattern->Noteblock[wholenote].EffectParam = XMPatternHeader->PatternData[i];
                i++;
            }

            wholenote++;    // get ready for the next note

        } // end of patterndata

        // if the pattern contains data...
        if (XMPatternHeader->PackedPatterndataLength > 0)
        {
            // ... check if the pattern contained all the notes it should!
            if (wholenote != ((Module->PatternLength[CurrentPattern]) * (Module->NumberofChannels)))
            {
                Module->NumberofPatterns = CurrentPattern + 1;
                Module->NumberofInstruments = 0;
                Module->State = XM7_STATE_ERROR | XM7_ERR_INCOMPLETE_PATTERN;
                return (XM7_ERR_INCOMPLETE_PATTERN);
            }
        }

        // get ready for next pattern!
        XMPatternHeader = (XM7_XMPatternHeader_Type*) &(XMPatternHeader->PatternData[i]);

    }    // end 'pattern' for

    // BETA TEST
    // return (0);

    // patterns are finished

    // set all the instrumen pointer to NULL
    u16 CurrentInstrument;
    for (CurrentInstrument = 0; CurrentInstrument < 128; CurrentInstrument++)
        Module->Instrument[CurrentInstrument] = NULL;

    // let's load the instruments!
    XM7_XMInstrument1stHeader_Type *XMInstrument1Header = (XM7_XMInstrument1stHeader_Type*) XMPatternHeader;

    // BETA TEST
    // Module->NumberofInstruments=1;

    for (CurrentInstrument = 0; CurrentInstrument < ((Module->NumberofInstruments)); CurrentInstrument++)
    {

        // check if the INSTRUMENT header is ok  (I'm unsure of the header length...)
        // NOTE: I've found some XM with HeaderLength!=0x107 and Type!=0 (Type=80) so I'm trashing the following check...
        //           ... then found also type=anything which has meaning so really don't trust this 'Type' field
        /*
         if ((XMInstrument1Header->HeaderLength!=0x107) && (XMInstrument1Header->Type!=0)) {
         Module->State = STATE_ERROR | ERR_UNSUPPORTED_INSTRUMENT_HEADER;
         return (ERR_UNSUPPORTED_INSTRUMENT_HEADER);
         }
         */

        // check if the INSTRUMENT lenght is ok  (max 16 samples!) (and can be ZERO!)
        if (XMInstrument1Header->NumberofSamples > 16)
        {
            Module->NumberofInstruments = CurrentInstrument;
            Module->State = XM7_STATE_ERROR | XM7_ERR_UNSUPPORTED_INSTRUMENT_HEADER;
            return (XM7_ERR_UNSUPPORTED_INSTRUMENT_HEADER);
        }

        // allocate the new instrument
        Module->Instrument[CurrentInstrument] = PrepareNewInstrument();
        if (Module->Instrument[CurrentInstrument] == NULL)
        {
            Module->NumberofInstruments = CurrentInstrument;
            Module->State = XM7_STATE_ERROR | XM7_ERR_NOT_ENOUGH_MEMORY;
            return (XM7_ERR_NOT_ENOUGH_MEMORY);
        }

        XM7_Instrument_Type *CurrentInstrumentPtr = Module->Instrument[CurrentInstrument];

        // load the info from the header
        CurrentInstrumentPtr->NumberofSamples = XMInstrument1Header->NumberofSamples;
        memcpy(CurrentInstrumentPtr->Name, XMInstrument1Header->Name, 22);        // char[22]

        if (XMInstrument1Header->NumberofSamples > 0)
        {
            // get the 2nd part of the header
            XM7_XMInstrument2ndHeader_Type *XMInstrument2Header = (XM7_XMInstrument2ndHeader_Type*) &(XMInstrument1Header->NextHeaderPart[0]);

            // 2009! HNY!
            // check the length of the instrument header before proceed!

            if (XMInstrument1Header->InstrumentHeaderLength >= 33 + 96)
            {
                // copy the 96 notes' sample numbers
                memcpy(CurrentInstrumentPtr->SampleforNote, XMInstrument2Header->SampleforNotes, 96);    // u8[96]
            }
            else
            {
                // fill with 0 all the samples numbers
                memset(CurrentInstrumentPtr->SampleforNote, 0, 96);
            }

            // if (XMInstrument1Header->InstrumentHeaderLength >= 33+96+123) {
            if (XMInstrument1Header->InstrumentHeaderLength >= 33 + 96 + 123 - 22)
            {
                // read all the data about Volume&Envelope points (the 22 'reserved' bytes can be absent)
                memcpy((u8*) CurrentInstrumentPtr->VolumeEnvelopePoint, (u8*) XMInstrument2Header->VolumeEnvelopePoints, 48); // 12x2x2=u8[48]
                memcpy((u8*) CurrentInstrumentPtr->PanningEnvelopePoint, (u8*) XMInstrument2Header->PanningEnvelopePoints, 48); // 12x2x2=u8[48]

                CurrentInstrumentPtr->NumberofVolumeEnvelopePoints = XMInstrument2Header->NumberofVolumePoints;
                CurrentInstrumentPtr->NumberofPanningEnvelopePoints = XMInstrument2Header->NumberofPanningPoints;

                CurrentInstrumentPtr->VolumeSustainPoint = XMInstrument2Header->VolumeSustainPoint;
                CurrentInstrumentPtr->VolumeLoopStartPoint = XMInstrument2Header->VolumeLoopStartPoint;
                CurrentInstrumentPtr->VolumeLoopEndPoint = XMInstrument2Header->VolumeLoopEndPoint;

                CurrentInstrumentPtr->PanningSustainPoint = XMInstrument2Header->PanningSustainPoint;
                CurrentInstrumentPtr->PanningLoopStartPoint = XMInstrument2Header->PanningLoopStartPoint;
                CurrentInstrumentPtr->PanningLoopEndPoint = XMInstrument2Header->PanningLoopEndPoint;

                CurrentInstrumentPtr->VolumeType = XMInstrument2Header->VolumeType;        //  bit 0: On; 1: Sustain; 2: Loop
                CurrentInstrumentPtr->PanningType = XMInstrument2Header->PanningType;  //  bit 0: On; 1: Sustain; 2: Loop

                // Instrument Vibrato
                CurrentInstrumentPtr->VibratoType = XMInstrument2Header->VibratoType;
                CurrentInstrumentPtr->VibratoSweep = 0x10000 / (XMInstrument2Header->VibratoSweep + 1);
                CurrentInstrumentPtr->VibratoDepth = XMInstrument2Header->VibratoDepth;
                CurrentInstrumentPtr->VibratoRate = XMInstrument2Header->VibratoRate;

                // envelope volume fadeout
                CurrentInstrumentPtr->VolumeFadeout = XMInstrument2Header->VolumeFadeOut;
            }
            else
            {
                // there are NO envelopes in the file...
                CurrentInstrumentPtr->VolumeType = 0;
                CurrentInstrumentPtr->PanningType = 0;
                CurrentInstrumentPtr->NumberofVolumeEnvelopePoints = 0;
                CurrentInstrumentPtr->NumberofPanningEnvelopePoints = 0;
                // if there's no vibrato in the file...
                CurrentInstrumentPtr->VibratoType = 0;
                CurrentInstrumentPtr->VibratoSweep = 0;
                CurrentInstrumentPtr->VibratoDepth = 0;
                CurrentInstrumentPtr->VibratoRate = 0;
                // if there's no fadeout in the file...
                CurrentInstrumentPtr->VolumeFadeout = 0;
            }

            // InstrumentHeader (2nd part) is finished!
            XM7_XMSampleHeader_Type *XMSampleHeader = (XM7_XMSampleHeader_Type*) ((u8*) &XMInstrument1Header->InstrumentHeaderLength + XMInstrument1Header->InstrumentHeaderLength);

            u8 CurrentSample;

            // read all the sample headers
            for (CurrentSample = 0; CurrentSample < (CurrentInstrumentPtr->NumberofSamples); CurrentSample++)
            {

                // allocate the new Sample
                CurrentInstrumentPtr->Sample[CurrentSample] = PrepareNewSample(XMSampleHeader->Length, XMSampleHeader->LoopLength, XMSampleHeader->Type);
                if (CurrentInstrumentPtr->Sample[CurrentSample] == NULL)
                {
                    Module->NumberofInstruments = CurrentInstrument + 1;
                    CurrentInstrumentPtr->NumberofSamples = CurrentSample;
                    Module->State = XM7_STATE_ERROR | XM7_ERR_NOT_ENOUGH_MEMORY;
                    return (XM7_ERR_NOT_ENOUGH_MEMORY);
                }

                XM7_Sample_Type *CurrentSamplePtr = CurrentInstrumentPtr->Sample[CurrentSample];

                // read all the data
                CurrentSamplePtr->LoopStart = XMSampleHeader->LoopStart;
                CurrentSamplePtr->LoopLength = XMSampleHeader->LoopLength;
                CurrentSamplePtr->Volume = XMSampleHeader->Volume;
                CurrentSamplePtr->FineTune = XMSampleHeader->FineTune;
                CurrentSamplePtr->Flags = XMSampleHeader->Type;
                // if loop type is 0x03 it becomes 'forward', because XMs shouldn't support 0x03 loops...
                if ((CurrentSamplePtr->Flags & 0x0F) == 0x03) CurrentSamplePtr->Flags = (CurrentSamplePtr->Flags & 0xF0) | 0x01;
                // end
                CurrentSamplePtr->Panning = XMSampleHeader->Panning;
                CurrentSamplePtr->RelativeNote = XMSampleHeader->RelativeNote;
                memcpy(CurrentSamplePtr->Name, XMSampleHeader->Name, 22);        // char[22]

                // Warn about potential issues with sample tuning (if too short)
                if (CurrentSamplePtr->Flags > 0 && CurrentSamplePtr->Length < 64) iprintf("ins %d, smpl %d too short\n", CurrentInstrument + 1, CurrentSample + 1);

                // point to the next sample header (or the 1st byte after all the headers...)
                XMSampleHeader = (XM7_XMSampleHeader_Type*) &(XMSampleHeader->NextHeader[0]);
            }

            // the 1st byte after the header(s)... (both for the 8 and the 16 bit samples...)
            XM7_SampleData_Type *SampleData = (XM7_SampleData_Type*) XMSampleHeader;
            XM7_SampleData16_Type *SampleData16 = (XM7_SampleData16_Type*) XMSampleHeader;

            // read all the sample data
            for (CurrentSample = 0; CurrentSample < (CurrentInstrumentPtr->NumberofSamples); CurrentSample++)
            {

                // get a pointer to the data space
                XM7_Sample_Type *CurrentSamplePtr = CurrentInstrumentPtr->Sample[CurrentSample];
                XM7_SampleData_Type *CurrentSampleDataPtr = (XM7_SampleData_Type*) CurrentSamplePtr->SampleData;
                XM7_SampleData16_Type *CurrentSampleData16Ptr = (XM7_SampleData16_Type*) CurrentSamplePtr->SampleData;

                // read the sample, finally!

                // check if sample is 8 or 16 bit first!
                u32 i, j;
                if ((CurrentSamplePtr->Flags & 0x10) == 0)
                {
                    // it's an 8 bit sample
                    s8 old = 0;
                    for (i = 0; i < CurrentSamplePtr->Length; i++)
                    {
                        // samples in XM files are stored as delta value (nobody knows why...)
                        old += SampleData->Data[i];
                        CurrentSampleDataPtr->Data[i] = old;
                    }

                    // ping-pong loop conversion START
                    // since DS has got no support for ping/pong loop, we should duplicate the loop backward
                    if ((CurrentSamplePtr->Flags & 0x03) == 0x02)
                    {

                        /*
                         for (j=0;j<(CurrentSamplePtr->LoopLength-2);j++) {
                         CurrentSampleDataPtr->Data[CurrentSamplePtr->Length+j] = CurrentSampleDataPtr->Data[(CurrentSamplePtr->Length-1)-j];
                         }
                         */

                        for (j = 0; j < CurrentSamplePtr->LoopLength; j++)
                            CurrentSampleDataPtr->Data[CurrentSamplePtr->Length + j] = CurrentSampleDataPtr->Data[CurrentSamplePtr->Length - j];

                        // and change it to a 'normal' loop (preserving 16 bit flag)
                        CurrentSamplePtr->Flags = (CurrentSamplePtr->Flags & 0xF0) | 0x01;

                        // the lenght of the sample must be changed
                        // CurrentSamplePtr->Length += (CurrentSamplePtr->LoopLength - 2);
                        CurrentSamplePtr->Length += CurrentSamplePtr->LoopLength;

                        // of course also LoopLength has to be changed!
                        CurrentSamplePtr->LoopLength += CurrentSamplePtr->LoopLength;
                    }
                    // ping-pong loop conversion FINISHED

                    // prepare for the next sample (both pointers!)
                    SampleData = (XM7_SampleData_Type*) &(SampleData->Data[i]);
                    SampleData16 = (XM7_SampleData16_Type*) SampleData;

                }
                else
                {
                    // it's a 16 bit sample
                    s16 old16 = 0;
                    for (i = 0; (i < CurrentSamplePtr->Length >> 1); i++) // length is in bytes anyway
                    {
                        old16 += SampleData16->Data[i];
                        CurrentSampleData16Ptr->Data[i] = old16;
                    }

                    // ping-pong loop conversion START
                    // since DS has got no support for ping/pong loop, we should duplicate the loop backward
                    if ((CurrentSamplePtr->Flags & 0x03) == 0x02)
                    {

                        /*
                         for (j=0;j<((CurrentSamplePtr->LoopLength >> 1) -2);j++) {  // -2 because we won't duplicate 1st and last
                         CurrentSampleData16Ptr->Data[(CurrentSamplePtr->Length >> 1) +j] = CurrentSampleData16Ptr->Data[(CurrentSamplePtr->Length >> 1)-(j+1)];
                         }
                         */

                        for (j = 0; j < (CurrentSamplePtr->LoopLength >> 1); j++)
                            CurrentSampleData16Ptr->Data[(CurrentSamplePtr->Length >> 1) + j] = CurrentSampleData16Ptr->Data[(CurrentSamplePtr->Length >> 1) - j];

                        // and change it to a 'normal' loop (preserving 16 bit flag)
                        CurrentSamplePtr->Flags = (CurrentSamplePtr->Flags & 0xF0) | 0x01;

                        // the lenght of the sample must be changed (it's in bytes)
                        // CurrentSamplePtr->Length += (CurrentSamplePtr->LoopLength - 4);
                        CurrentSamplePtr->Length += CurrentSamplePtr->LoopLength;

                        // of course also LoopLength has to be changed! (it's in bytes)
                        // CurrentSamplePtr->LoopLength += (CurrentSamplePtr->LoopLength - 4);
                        CurrentSamplePtr->LoopLength += CurrentSamplePtr->LoopLength;
                    }
                    // ping-pong loop conversion FINISHED

                    // prepare for the next sample (both pointers!)
                    SampleData16 = (XM7_SampleData16_Type*) &(SampleData16->Data[i]);
                    SampleData = (XM7_SampleData_Type*) SampleData16;
                }

            }       // finished reading all the samples

            // prepare for the next instrument (the next byte after the last sample
            XMInstrument1Header = (XM7_XMInstrument1stHeader_Type*) SampleData;

        }   // end if    (samples>0)
        else
        {
            // if (samples=0)
            // should be like this...
            // XMInstrument1Header = (XMInstrument1stHeader_Type*) &(XMInstrument1Header->NextHeaderPart[0]);

            // ...but it seems like THERE IS the 2nd header too even when NO samples...
            // get the 2nd part of the header

            // NOTE: I've found some XM with instruments with 0 samples so I trash the following 2 lines
            /*
             XMInstrument2ndHeader_Type* XMInstrument2Header = (XMInstrument2ndHeader_Type*) &(XMInstrument1Header->NextHeaderPart[0]);
             XMInstrument1Header = (XMInstrument1stHeader_Type*) &(XMInstrument2Header->NextDataPart[0]);
             */

            XMInstrument1Header =
                    (XM7_XMInstrument1stHeader_Type*) &(XMInstrument1Header->NextHeaderPart[(XMInstrument1Header->InstrumentHeaderLength) - (sizeof(XM7_XMInstrument1stHeader_Type)) + 1]);

        }
    }    // end "for Instruments"

    // Set standard panning
    Module->AmigaPanningEmulation = XM7_PANNING_TYPE_NORMAL;
    Module->AmigaPanningDisplacement = 0x00;

    // Set volume
    Module->CurrentGlobalVolume = 0x40;

    // set State
    Module->State = XM7_STATE_READY;

    // Set Current Song Position and Pattern Number
    Module->CurrentSongPosition = 0;
    Module->CurrentPatternNumber = Module->PatternOrder[0];

    // By default, no note transposing is applied
    Module->Transpose = 0;
    // By default, no loop
    Module->LoopMode = 0;
    // At startup, current line is 0
    Module->CurrentLine = 0;
    // Set to false by default
    Module->bGotoHotCue = FALSE;

    // end OK!
    return (0);
}

void XM7_UnloadXM(XM7_ModuleManager_Type *Module)
{
    s16 i, j;
    XM7_Instrument_Type *CurrentInstrumentPtr;
    XM7_Sample_Type *CurrentSamplePtr;

    // instruments, from last to first
    for (i = (Module->NumberofInstruments - 1); i >= 0; i--)
    {
        CurrentInstrumentPtr = Module->Instrument[i];
		
		if (CurrentInstrumentPtr == NULL)
			continue;
		
        // samples, from last to first
        for (j = (CurrentInstrumentPtr->NumberofSamples - 1); j >= 0; j--)
        {
            CurrentSamplePtr = CurrentInstrumentPtr->Sample[j];
			
			if (CurrentSamplePtr == NULL)  // Safety
				continue;
		
            // remove sample data
            free(CurrentSamplePtr->SampleData);
            // remove sample info
            free(CurrentSamplePtr);
        }
        // remove instrument
        free(CurrentInstrumentPtr);
    }
    // remove patterns
    for (i = (Module->NumberofPatterns - 1); i >= 0; i--)
        free(Module->Pattern[i]);

    Module->State = XM7_STATE_EMPTY;
}

void XM7_SetPanningStyle(XM7_ModuleManager_Type *Module, u8 style, u8 displacement)
{
    Module->AmigaPanningEmulation = style;
    Module->AmigaPanningDisplacement = displacement;
}
