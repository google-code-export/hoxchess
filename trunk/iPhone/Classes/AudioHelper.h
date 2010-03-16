/***************************************************************************
 *  Copyright 2009-2010 Nevo Hua  <nevo.hua@playxiangqi.com>               *
 *                                                                         * 
 *  This file is part of NevoChess.                                        *
 *                                                                         *
 *  NevoChess is free software: you can redistribute it and/or modify      *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, either version 3 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  NevoChess is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with NevoChess.  If not, see <http://www.gnu.org/licenses/>.     *
 ***************************************************************************/

#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

#define kNumberBuffers 3

// ---------------------------------------------------
@interface AudioHelper : NSObject
{
    BOOL                 _enabled;
    NSMutableDictionary* _loadedSounds;
}

@property BOOL enabled;

+ (AudioHelper*) sharedInstance;
- (void) playSound:(NSString*)sound;
@end


// ---------------------------------------------------
@interface AudioData : NSObject
{
    UInt32                       bufferByteSize; // the number of bytes to use in each audio queue buffer
	UInt32                       numPacketsToRead; // the number of audio data packets to read into each audio queue buffer
	
	Float32                      gain; // the gain (relative audio level) for the playback audio queue
    
	AudioQueueRef                mQueue;
	AudioQueueBufferRef          mBuffers[kNumberBuffers];  

    AudioFileID                  audioFileID; // the identifier for the audio file to play
	AudioStreamBasicDescription  audioFormat;
	AudioQueueLevelMeterState*   audioLevels;
	SInt64                       startingPacketNumber; // the current packet number in the playback file
}

- (id)initWithSoundFile:(NSString*)path;

- (void)prepareAudioData:(CFURLRef)url;
- (void)play;
- (void)enableLevelMetering;
- (void)calculateSizesFor:(Float64)seconds;

@property(readwrite) UInt32  bufferByteSize;   // the number of bytes to use in each audio queue buffer
@property(readwrite) UInt32  numPacketsToRead; // the number of audio data packets to read into each audio queue buffer

@property(readwrite) Float32 gain; // the gain (relative audio level) for the playback audio queue

@property(readwrite)  AudioQueueRef	mQueue;  

@property(readwrite) AudioFileID                 audioFileID; // the identifier for the audio file to play
@property(readwrite) AudioStreamBasicDescription audioFormat;
@property(readwrite) AudioQueueLevelMeterState  *audioLevels;
@property(readwrite) SInt64		startingPacketNumber; // the current packet number in the playback file

@end
