#import <Cocoa/Cocoa.h>
#include "framework.h"

@interface AppDelegate : NSObject<NSApplicationDelegate,NSWindowDelegate>
@property(nonatomic,retain) NSWindow *window;
@end

@implementation AppDelegate
-(void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,500,500) styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable | NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
    NSWindowController *controller = [[NSWindowController alloc] initWithWindow:window];
    [window center];
    [window layoutIfNeeded];
    [controller showWindow:self];
    importantFunction();
}

-(void)applicationWillTerminate:(NSNotification *)notification {

}
@end


int main(int argc,char *const argv[]){

    [NSApplication sharedApplication];
    [NSApp setDelegate:[[AppDelegate alloc] init]];
    [NSApp run];


    return 0;
};