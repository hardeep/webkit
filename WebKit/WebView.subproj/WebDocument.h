/*	
    WebDocument.h
    Copyright 2002, Apple, Inc. All rights reserved.
*/

#import <Cocoa/Cocoa.h>

@class WebDataSource;
@class WebError;

/*!
    @protocol WebDocumentView
    @discussion Protocol implemented by the document view of WebView
*/
@protocol WebDocumentView <NSObject>

/*!
    @method setDataSource:
    @abstract Called when the corresponding data source has been created.
    @param dataSource The corresponding data source.
*/
- (void)setDataSource: (WebDataSource *)dataSource;

/*!
    @method dataSourceUpdated:
    @abstract Called when the corresponding data source has received data.
    @param dataSource The corresponding data source.
*/
- (void)dataSourceUpdated: (WebDataSource *)dataSource;

/*!
    @method setNeedsLayout:
    @discussion Called when WebKit has determined that the document view needs to layout.
    This method should simply set a flag and call layout from drawRect if the flag is YES.
    @param flag YES to cause a layout, no to not cause a layout.
*/
- (void)setNeedsLayout: (BOOL)flag;

/*!
    @method layout
    @discussion Called when the document view must immediately layout. For simple views,
    setting the frame is a sufficient implementation of this method.
*/
- (void)layout;
@end


/*!
    @protocol WebDocumentDragSettings
    @discussion Optional protocol for setting dragging behavior for the document view of WebView. 
*/
@protocol WebDocumentDragSettings <NSObject>
/*!
    @method setAcceptsDrags:
    @abstract Sets whether one can drag from the document view or not.
    @param flag YES allows drags from the document view, NO disallows drags from the document view.
*/
- (void)setAcceptsDrags: (BOOL)flag;

/*!
    @method acceptsDrags
    @result Whether one can drag from the document view or not.
*/
- (BOOL)acceptsDrags;

/*!
    @method setAcceptsDrops:
    @abstract Sets whether one can drag to the document view or not.
    @param flag YES allows drags to the document view, NO disallows drags to the document view.
*/
- (void)setAcceptsDrops: (BOOL)flag;

/*!
    @method acceptsDrops
    @result Whether one can drag to the document view or not.
*/
- (BOOL)acceptsDrops;
@end


/*!
    @protocol WebDocumentSearching
    @discussion Optional protocol for searching document view of WebView. 
*/
@protocol WebDocumentSearching <NSObject>
/*!
    @method searchFor:direction:caseSensitive:
    @abstract Searches a document view for a string and highlights the string if it is found.
    @param string The string to search for.
    @param forward YES to search forward, NO to seach backwards.
    @param caseFlag YES to for case-sensitive search, NO for case-insensitive search.
    @result YES if found, NO if not found.
*/
- (BOOL)searchFor: (NSString *)string direction: (BOOL)forward caseSensitive: (BOOL)caseFlag;
@end


/*!
    @protocol WebDocumentTextEncoding
    @discussion Optional protocol for supporting text encoding.
*/
@protocol WebDocumentText <NSObject>

/*!
    @method supportsTextEncoding
    @result YES if the document view support text encoding, NO if it doesn't.
*/
- (BOOL)supportsTextEncoding;

/*!
    @method string
    @result String that represents the entire document.
*/
- (NSString *)string;

/*!
    @method attributedString
    @result Attributed string that represents the entire document.
*/
- (NSAttributedString *)attributedString;

/*!
    @method selectedString
    @result String that represents the current selection.
*/
- (NSString *)selectedString;

/*!
    @method selectedAttributedString
    @result Attributed string that represents the current selection.
*/
- (NSAttributedString *)selectedAttributedString;


/*!
    @method selectAll
    @abstract Selects all the text in the document.
*/
- (void)selectAll;

/*!
    @method deselectText
    @abstract Causes a text selection to lose its selection.
*/
- (void)deselectAll;

@end


/*!
    @protocol WebDocumentRepresentation
    @discussion Protocol implemented by the document representation of a data source.
*/
@protocol WebDocumentRepresentation <NSObject>
/*!
    @method setDataSource:
    @abstract Called soon after the document representation is created.
    @param dataSource The data source that is set.
*/
- (void)setDataSource: (WebDataSource *)dataSource;

/*!
    @method receivedData:withDataSource:
    @abstract Called when the data source has received data.
    @param data The data that the data source has received.
    @param dataSource The data source that has received data.
*/
- (void)receivedData: (NSData *)data withDataSource: (WebDataSource *)dataSource;

/*!
    @method receivedError:withDataSource:
    @abstract Called when the data source has received an error.
    @param error The error that the data source has received.
    @param dataSource The data source that has received the error.
*/
- (void)receivedError: (WebError *)error withDataSource: (WebDataSource *)dataSource;

/*!
    @method finishLoadingWithDataSource:
    @abstract Called when the data source has finished loading.
    @param dataSource The datasource that has finished loading.
*/
- (void)finishedLoadingWithDataSource: (WebDataSource *)dataSource;

/*!
    @method canProvideDocumentSource
    @return Returns true if the representation can provide document source.
*/
- (BOOL)canProvideDocumentSource;

/*!
    @method documentSource
    @return Returns the textual source representation of the document.  For HTML documents
    this is the original HTML source.
*/
- (NSString *)documentSource;
@end
