//
//  WebDOMNode.m
//  WebKit
//
//  Created by Darin Adler on Tue Jun 11 2002.
//  Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
//

#import "WebDOMNode.h"

#import <WebKit/WebView.h>
#import <WebKit/WebBridge.h>
#import <WebKit/WebHTMLViewPrivate.h>

@interface WebKitDOMTreeCopier : NSObject <WebCoreDOMTreeCopier>
@end

@implementation WebDOMNode

- initWithName:(NSString *)n value:(NSString *)v source:(NSString *)s children:(NSArray *)c
{
    [super init];

    children = [c copy];
    name = [n copy];
    value = [v copy];
    source = [s copy];
    
    return self;
}

- initWithWebView:(WebView *)view
{
    WebKitDOMTreeCopier *copier;
    
    [self dealloc];

    if (![[view documentView] isMemberOfClass:[WebHTMLView class]]) {
        return nil;
    }
    
    copier = [[WebKitDOMTreeCopier alloc] init];
    WebHTMLView *htmlView = (WebHTMLView *)[view documentView];
    self = [[[htmlView _bridge] copyDOMTree:copier] retain];
    [copier release];
    
    return self;
}

- (void)dealloc
{
    [children release];
    [name release];
    [value release];
    [source release];
    
    [super dealloc];
}

- (NSArray *)children
{
    return children;
}

- (NSString *)name
{
    return name;
}

- (NSString *)value
{
    return value;
}

- (NSString *)source
{
    return source;
}

@end

@implementation WebKitDOMTreeCopier

- (NSObject *)nodeWithName:(NSString *)n value:(NSString *)v source:(NSString *)s children:(NSArray *)c
{
    return [[[WebDOMNode alloc] initWithName:n value:v source:s children:c] autorelease];
}

@end
