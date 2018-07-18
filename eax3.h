/*              DirectSound
 *
 * Copyright 2018 Chris Robinson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef EAX3_H
#define EAX3_H

#ifndef EAXVECTOR_DEFINED
#define EAXVECTOR_DEFINED
typedef struct _EAXVECTOR {
    float x, y, z;
} EAXVECTOR;
#endif

DEFINE_GUID(DSPROPSETID_EAX30_ListenerProperties, 0xa8fa6882, 0xb476, 0x11d3, 0xbd, 0xb9, 0x00, 0xc0, 0xf0, 0x2d, 0xdf, 0x87);
typedef enum {
    DSPROPERTY_EAX30LISTENER_NONE,
    DSPROPERTY_EAX30LISTENER_ALLPARAMETERS,
    DSPROPERTY_EAX30LISTENER_ENVIRONMENT,
    DSPROPERTY_EAX30LISTENER_ENVIRONMENTSIZE,
    DSPROPERTY_EAX30LISTENER_ENVIRONMENTDIFFUSION,
    DSPROPERTY_EAX30LISTENER_ROOM,
    DSPROPERTY_EAX30LISTENER_ROOMHF,
    DSPROPERTY_EAX30LISTENER_ROOMLF,
    DSPROPERTY_EAX30LISTENER_DECAYTIME,
    DSPROPERTY_EAX30LISTENER_DECAYHFRATIO,
    DSPROPERTY_EAX30LISTENER_DECAYLFRATIO,
    DSPROPERTY_EAX30LISTENER_REFLECTIONS,
    DSPROPERTY_EAX30LISTENER_REFLECTIONSDELAY,
    DSPROPERTY_EAX30LISTENER_REFLECTIONSPAN,
    DSPROPERTY_EAX30LISTENER_REVERB,
    DSPROPERTY_EAX30LISTENER_REVERBDELAY,
    DSPROPERTY_EAX30LISTENER_REVERBPAN,
    DSPROPERTY_EAX30LISTENER_ECHOTIME,
    DSPROPERTY_EAX30LISTENER_ECHODEPTH,
    DSPROPERTY_EAX30LISTENER_MODULATIONTIME,
    DSPROPERTY_EAX30LISTENER_MODULATIONDEPTH,
    DSPROPERTY_EAX30LISTENER_AIRABSORPTIONHF,
    DSPROPERTY_EAX30LISTENER_HFREFERENCE,
    DSPROPERTY_EAX30LISTENER_LFREFERENCE,
    DSPROPERTY_EAX30LISTENER_ROOMROLLOFFFACTOR,
    DSPROPERTY_EAX30LISTENER_FLAGS
} DSPROPERTY_EAX30_LISTENERPROPERTY;

/* Stores the value being set, but does not apply it */
#define DSPROPERTY_EAX30LISTENER_DEFERRED               0x80000000
/* The lack of the deferred flag forces a call to CommitDeferredSettings(),
 * applying *all* deferred settings, including the EAX property being set */
#define DSPROPERTY_EAX30LISTENER_IMMEDIATE              0x00000000
/* Same as IMMEDIATE; causes a commit of deferred properties but implies no
 * extra property being set */
#define DSPROPERTY_EAX30LISTENER_COMMITDEFERREDSETTINGS 0x00000000

/* DSPROPERTY_EAX30LISTENER_ALLPARAMETERS */
typedef EAXREVERBPROPERTIES EAX30LISTENERPROPERTIES, *LPEAX30LISTENERPROPERTIES;

/* DSPROPERTY_EAX30LISTENER_FLAGS */
/* These flags determine what properties are modified when the environment size
 * is changed.
 */
#define EAX30LISTENERFLAGS_DECAYTIMESCALE        0x00000001
#define EAX30LISTENERFLAGS_REFLECTIONSSCALE      0x00000002
#define EAX30LISTENERFLAGS_REFLECTIONSDELAYSCALE 0x00000004
#define EAX30LISTENERFLAGS_REVERBSCALE           0x00000008
#define EAX30LISTENERFLAGS_REVERBDELAYSCALE      0x00000010
/* This flag limits the high frequency decay according to air absorption */
#define EAX30LISTENERFLAGS_DECAYHFLIMIT          0x00000020
#define EAX30LISTENERFLAGS_ECHOTIMESCALE         0x00000040
#define EAX30LISTENERFLAGS_MODTIMESCALE          0x00000080


DEFINE_GUID(DSPROPSETID_EAX30_BufferProperties, 0xa8fa6881, 0xb476, 0x11d3, 0xbd, 0xb9, 0x00, 0xc0, 0xf0, 0x2d, 0xdf, 0x87);
typedef enum {
    DSPROPERTY_EAX30BUFFER_NONE,
    DSPROPERTY_EAX30BUFFER_ALLPARAMETERS,
    DSPROPERTY_EAX30BUFFER_OBSTRUCTIONPARAMETERS,
    DSPROPERTY_EAX30BUFFER_OCCLUSIONPARAMETERS,
    DSPROPERTY_EAX30BUFFER_EXCLUSIONPARAMETERS,
    DSPROPERTY_EAX30BUFFER_DIRECT,
    DSPROPERTY_EAX30BUFFER_DIRECTHF,
    DSPROPERTY_EAX30BUFFER_ROOM,
    DSPROPERTY_EAX30BUFFER_ROOMHF,
    DSPROPERTY_EAX30BUFFER_OBSTRUCTION,
    DSPROPERTY_EAX30BUFFER_OBSTRUCTIONLFRATIO,
    DSPROPERTY_EAX30BUFFER_OCCLUSION,
    DSPROPERTY_EAX30BUFFER_OCCLUSIONLFRATIO,
    DSPROPERTY_EAX30BUFFER_OCCLUSIONROOMRATIO,
    DSPROPERTY_EAX30BUFFER_OCCLUSIONDIRECTRATIO,
    DSPROPERTY_EAX30BUFFER_EXCLUSION,
    DSPROPERTY_EAX30BUFFER_EXCLUSIONLFRATIO,
    DSPROPERTY_EAX30BUFFER_OUTSIDEVOLUMEHF,
    DSPROPERTY_EAX30BUFFER_DOPPLERFACTOR,
    DSPROPERTY_EAX30BUFFER_ROLLOFFFACTOR,
    DSPROPERTY_EAX30BUFFER_ROOMROLLOFFFACTOR,
    DSPROPERTY_EAX30BUFFER_AIRABSORPTIONFACTOR,
    DSPROPERTY_EAX30BUFFER_FLAGS
} DSPROPERTY_EAX30_BUFFERPROPERTY;

#define DSPROPERTY_EAX30BUFFER_DEFERRED               0x80000000
/* NOTE: This applies all deferred changes, not just the buffer's. */
#define DSPROPERTY_EAX30BUFFER_IMMEDIATE              0x00000000
#define DSPROPERTY_EAX30BUFFER_COMMITDEFERREDSETTINGS 0x00000000

/* DSPROPERTY_EAX30BUFFER_ALLPARAMETERS */
typedef EAXSOURCEPROPERTIES EAX30BUFFERPROPERTIES, *LPEAX30BUFFERPROPERTIES;

/* DSPROPERTY_EAX30BUFFER_OBSTRUCTION */
#ifndef EAX_OBSTRUCTIONPROPERTIES_DEFINED
#define EAX_OBSTRUCTIONPROPERTIES_DEFINED
typedef struct _EAXOBSTRUCTIONPROPERTIES {
    long  lObstruction;
    float flObstructionLFRatio;
} EAXOBSTRUCTIONPROPERTIES, *LPEAXOBSTRUCTIONPROPERTIES;
#endif

/* DSPROPERTY_EAX30BUFFER_OCCLUSION */
#ifndef EAX_OCCLUSIONPROPERTIES_DEFINED
#define EAX_OCCLUSIONPROPERTIES_DEFINED
typedef struct _EAXOCCLUSIONPROPERTIES {
    long  lOcclusion;
    float flOcclusionLFRatio;
    float flOcclusionRoomRatio;
    float flOcclusionDirectRatio;
} EAXOCCLUSIONPROPERTIES, *LPEAXOCCLUSIONPROPERTIES;
#endif

/* DSPROPERTY_EAX30BUFFER_EXCLUSION */
#ifndef EAX_EXCLUSIONPROPERTIES_DEFINED
#define EAX_EXCLUSIONPROPERTIES_DEFINED
typedef struct _EAXEXCLUSIONPROPERTIES {
    long  lExclusion;
    float flExclusionLFRatio;
} EAXEXCLUSIONPROPERTIES, *LPEAXEXCLUSIONPROPERTIES;
#endif

/* DSPROPERTY_EAX30BUFFER_FLAGS */
#define EAX30BUFFERFLAGS_DIRECTHFAUTO 0x00000001
#define EAX30BUFFERFLAGS_ROOMAUTO     0x00000002
#define EAX30BUFFERFLAGS_ROOMHFAUTO   0x00000004

#endif /* EAX3_H */
