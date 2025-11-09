#import <AVFoundation/AVFoundation.h>

extern "C" void gx_enableBackgroundAudio_ios()
{
  @autoreleasepool {
    AVAudioSession *session = [AVAudioSession sharedInstance];
    NSError *err = nil;
    BOOL ok = [session setCategory:AVAudioSessionCategoryPlayback
                       withOptions:0
                       error:&error];
    if (!ok) { NSLog(@"AVAudioSession setCategory error: %@", err); }
    ok = [session setActive:YES error:&err];
    if (!ok) { NSLog(@"AVAudioSession setActive error: %@", err); }
  }
}
