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

#import "AudioHelper.h"
#import "Enums.h"

// playback callback
static void playbackCallback(void*               inUserData,
                             AudioQueueRef       inAudioQueue,
                             AudioQueueBufferRef bufferReference)
{
	// This callback, being outside the implementation block, needs a reference to the AudioPlayer object
	AudioData *player = (AudioData *) inUserData;
	UInt32 numBytes;
	UInt32 numPackets = [player numPacketsToRead];	
	
	// This callback is called when the playback audio queue object has an audio queue buffer
	// available for filling with more data from the file being played
	AudioFileReadPackets( [player audioFileID],
						  NO,
						  &numBytes,
						  bufferReference->mPacketDescriptions,
						  [player startingPacketNumber],
						  &numPackets, 
						  bufferReference->mAudioData );
	
	if (numPackets > 0) {

		bufferReference->mAudioDataByteSize			= numBytes;		
		bufferReference->mPacketDescriptionCount	= numPackets;
		
		AudioQueueEnqueueBuffer(inAudioQueue, bufferReference, 0, NULL);
		player.startingPacketNumber = player.startingPacketNumber +  numPackets;
		
	} else {
        AudioQueueStop(inAudioQueue, NO);
        player.startingPacketNumber = 0; // always rewind to the start
        playbackCallback(inUserData, inAudioQueue, bufferReference);
	}
}

@implementation AudioData
@synthesize bufferByteSize;    // the number of bytes to use in each audio queue buffer
@synthesize numPacketsToRead;  // the number of audio data packets to read into each audio queue buffer
@synthesize gain;			   // the gain (relative audio level) for the playback audio queue
@synthesize mQueue;  
@synthesize audioFileID;	   // the identifier for the audio file to play
@synthesize audioFormat;
@synthesize audioLevels;
@synthesize startingPacketNumber;

- (void)dealloc
{
 	AudioQueueStop(mQueue, YES);
	AudioFileClose(audioFileID);
    AudioQueueDispose(mQueue, YES);
    [super dealloc];
}

- (id)initWithSoundFile:(NSString*)path
{
    if ([super init]) {
        CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)[path UTF8String], [path length], FALSE);
        if (url) {
            [self prepareAudioData:url];
            CFRelease(url);
        }
    }
    return self;
}

- (void)prepareAudioData:(CFURLRef)url
{
    AudioFileOpenURL( url,
					  0x01, // fsRdPerm (read only)
                      kAudioFileWAVEType,
					  &audioFileID );

	UInt32 s = sizeof([self audioFormat]);

	// get the AudioStreamBasicDescription format for the playback file
	AudioFileGetProperty( [self audioFileID], 
						  kAudioFilePropertyDataFormat,
						  &s,
						  &audioFormat );
    
    // create the playback audio queue object
	AudioQueueNewOutput( &audioFormat,
						 playbackCallback,
						 self, 
						 CFRunLoopGetCurrent (),
						 kCFRunLoopCommonModes,
						 0,  // run loop flags
						 &mQueue );
	
	// set the volume of the playback audio queue
	[self setGain: 1.0];
	
	AudioQueueSetParameter( mQueue,
							kAudioQueueParam_Volume,
							gain );
	
	[self enableLevelMetering];
    [self calculateSizesFor:0.5f]; // adjust to 0.5 seconds before starting
    // prime the queue with some data before starting
    for (int i = 0; i < kNumberBuffers; ++i) {
        AudioQueueAllocateBuffer(mQueue, bufferByteSize, &mBuffers[i]);
        playbackCallback(self, mQueue, mBuffers[i]);
    }	
}

- (void)play
{
    AudioQueueStart( mQueue, NULL /* start time. NULL means ASAP */ );
}

// an audio queue object doesn't provide audio level information unless you 
// enable it to do so
- (void) enableLevelMetering {
    
	// allocate the memory needed to store audio level information
	audioLevels = (AudioQueueLevelMeterState *) calloc (sizeof (AudioQueueLevelMeterState), audioFormat.mChannelsPerFrame);
    
	UInt32 trueValue = YES;
    
	AudioQueueSetProperty( mQueue,
                           kAudioQueueProperty_EnableLevelMetering,
                           &trueValue,
                           sizeof (UInt32) );
}

- (void) calculateSizesFor: (Float64) seconds {
	
	UInt32 maxPacketSize;
	UInt32 propertySize = sizeof (maxPacketSize);
	
	AudioFileGetProperty( audioFileID, 
						  kAudioFilePropertyPacketSizeUpperBound,
						  &propertySize,
						  &maxPacketSize );
	
	static const int maxBufferSize = 0x10000;  // limit maximum size to 64K
	static const int minBufferSize = 0x4000;   // limit minimum size to 16K
	
	if (audioFormat.mFramesPerPacket) {
		Float64 numPacketsForTime = audioFormat.mSampleRate / audioFormat.mFramesPerPacket * seconds;
		[self setBufferByteSize: numPacketsForTime * maxPacketSize];
	} else {
		// if frames per packet is zero, then the codec doesn't know the relationship between 
		// packets and time -- so we return a default buffer size
		[self setBufferByteSize: maxBufferSize > maxPacketSize ? maxBufferSize : maxPacketSize];
	}
	
	// we're going to limit our size to our default
	if (bufferByteSize > maxBufferSize && bufferByteSize > maxPacketSize) {
		[self setBufferByteSize: maxBufferSize];
	} else {
		// also make sure we're not too small - we don't want to go the disk for too small chunks
		if (bufferByteSize < minBufferSize) {
			[self setBufferByteSize: minBufferSize];
		}
	}
	
	[self setNumPacketsToRead: self.bufferByteSize / maxPacketSize];
}


@end

// ----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////
//
// CREDITS: 
//  http://stackoverflow.com/questions/145154/what-does-your-objective-c-singleton-look-like
//
//////////////////////////////////////////////////////////////////////////

#pragma mark -
#pragma mark The class (Singleton) instance object for Audio

static AudioHelper* _sharedAudio = nil;

#pragma mark -

@implementation AudioHelper

#pragma mark -
#pragma mark Singleton methods

+ (AudioHelper*) sharedInstance
{
    @synchronized(self)
    {
        if (_sharedAudio == nil) {
            _sharedAudio = [[AudioHelper alloc] init];
        }
    }
    return _sharedAudio;
}

+ (id) allocWithZone:(NSZone *)zone
{
    @synchronized(self) {
        if (_sharedAudio == nil) {
            _sharedAudio = [super allocWithZone:zone];
            return _sharedAudio;  // assignment and return on first allocation
        }
    }
    return nil; // on subsequent allocation attempts return nil
}

- (id) copyWithZone:(NSZone *)zone { return self; }
- (id) retain { return self; }
- (unsigned) retainCount { return UINT_MAX; /* ... cannot be released */ }
- (id) autorelease { return self; }


#pragma mark -
#pragma mark Normal methods

@synthesize enabled=_enabled;

- (id) init
{
    if (self = [super init])
    {
        _enabled = YES;
        _loadedSounds = [[NSMutableDictionary alloc] init];

        NSArray* soundList =
            [NSArray arrayWithObjects: @"CAPTURE", @"CAPTURE2", @"CLICK",
                                       @"DRAW", @"LOSS", @"CHECK2",
                                       @"MOVE", @"MOVE2", @"WIN", @"ILLEGAL",
                                       @"Check1", @"Replay", @"Undo", @"ChangeRole", 
                                       nil];
        for (NSString* sound in soundList)
        {
            NSString* path = [[NSBundle  mainBundle] pathForResource:sound ofType:@"WAV"
                                                         inDirectory:HC_SOUND_PATH];
            if (!path) {
                NSLog(@"%s: WARN: Failed to locate the sound file [%@].", __FUNCTION__, sound);
                continue;
            }
            AudioData* snd = [[AudioData alloc] initWithSoundFile:path];
            [_loadedSounds setObject:snd forKey:sound];
            [snd release];
        }
    }
    return self;
}

- (void) dealloc
{
    // NOTE: We actually do not need to do anything because the Singleton
    //       object will never be released.
    //       The following code is provided to avoid warnings from
    //       Xcode's Build and Analyze tool.

    [_loadedSounds release];
    [super dealloc];
}

- (void) playSound:(NSString*)sound
{
    if (_enabled) {
        AudioData* snd = [_loadedSounds objectForKey:sound];
        [snd play];
    }
}

@end

// we could always release the singleton instance safely
// here, but we might be able to do that in app delegate
// as well
__attribute__((__destructor__))
void releaseMySelfOnQuit() 
{
    [_sharedAudio release];
}
