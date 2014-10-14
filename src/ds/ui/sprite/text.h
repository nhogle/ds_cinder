#pragma once
#ifndef DS_UI_TEXT_H
#define DS_UI_TEXT_H
#include <cinder/gl/Texture.h>
#include <cinder/gl/TextureFont.h>
#include <cinder/Text.h>
#include <cinder/Font.h>
#include "ds/ui/service/render_text_service.h"
#include "ds/ui/sprite/sprite.h"
#include "ds/ui/sprite/text_layout.h"
#include "cinder/gl/Fbo.h"

//#define TEXT_RENDER_ASYNC		(1)

namespace ds {
namespace ui {
void clearFontCache();

/**
 * \class ds::ui::Text
 * A sprite for displaying text. All layouts are generated by a layout
 * function. See text_layout for all possible functions, or write your own.
 * Note that some functions require additional properties not on the sprite
 * (for example, spacing between lines, for multi-line layouts), in which
 * case the function is typically generated by an additional object responsible
 * for storing the info.
 */
class Text : public Sprite
{
public:
	Text(SpriteEngine&);
	~Text();

	// The comments below are a little obtuse. 90% of the time you'll want one of these:
	// 1. Single line of text at any width: use this Text, set the text, done!
	// 2. Multiple lines of text where you set the width and it wraps to any height: use MultilineText and call setResizeLimit(yourWidth, 0), set the text.
	// Forget about everything else. You're welcome, future me.

	// The text allows a lot of control between auto resizing and forcing specific
	// sizes. In general, resizeToText will do exactly what it says, and resize
	// the sprite after a layout to match the text. However, you can set a limit,
	// which will prevent the auto resize from going beyond a certain value.
	Text&						setResizeToText(const bool = true);
	Text&						setResizeToText(const bool width, const bool height);
	bool						autoResizeWidth() const;
	bool						autoResizeHeight() const;
	// When the limit is <= 0 it is unusued. When it's a valid value, then it is
	// ignored if resize is OFF, but if resize is ON, it prevents the view from
	// being resized beyond the limit.
	float						getResizeLimitWidth() const;
	float						getResizeLimitHeight() const;
	Text&						setResizeLimit(const float width = 0, const float height = 0);

	// Name can be a full filename, or a short name, if the FontList has been filled out.
	Text&						setFont(const std::string& name, const float fontSize);
	virtual void				setSizeAll(float width, float height, float depth);
	virtual float				getWidth() const;
	virtual float				getHeight() const;
	// This got messed up -- it's only implemented in the multiline subclass.
	//void						setAlignment( int alignment );
	Text&						setBorder(const ci::Rectf&);
	virtual void				updateClient(const UpdateParams &updateParams);
	virtual void				updateServer(const UpdateParams&);
	void						drawLocalClient();
	Text&						setText(const std::string &text);
	Text&						setText(const std::wstring &text);
	std::string					getTextAsString() const;
	std::wstring				getText() const;
	bool						hasText() const;

	// Set a function for translating a string into a layout object.
	Text&						setLayoutFunction(const TextLayout::MAKE_FUNC&);

	// Font metrics, this is probably temporary
	float						getFontAscent() const;
	float						getFontDescent() const;
	float						getFontHeight() const; // does not include leading
	float						getFontLeading() const;
	virtual float				getLeading() const;
	// Return the font height in pixels, including ascender and descender. Seems
	// like one of the other functions should do this, but they don't.
	float						getPixelFontHeight() const;
	// Hack in a function to get the full font height, including the leading
	virtual float				getFontFullHeight() const;

	// Print my line info
	void						debugPrint();

	const ds::ui::TextLayout&	getTextLayout() const { return mLayout; }

protected:
	virtual void				writeAttributesTo(ds::DataBuffer&);
	virtual void				readAttributeFrom(const char attributeId, ds::DataBuffer&);

	bool						mNeedsLayout;
	bool						mNeedRedrawing;

	FontPtr						mFont;

private:
	typedef Sprite inherited;

	void						makeLayout();
	void						drawIntoFbo();
	// Only used when ResizeToText is on
	void						calculateFrame(const int flags);

	void						onRenderFinished(RenderTextFinished&);

	//ci::gl::TextureFontRef	mTextureFont;
	//ci::gl::TextureFont::DrawOptions
	//							mDrawOptions;

	std::string					mFontFileName;
	float						mFontSize;
	std::wstring				mTextString;
	ci::Rectf					mBorder;

	// After performing the layout, this determines if my width and/or height
	// will be set to the layout bounds.
	int							mResizeToTextF;
	TextLayout					mLayout;
	TextLayout::MAKE_FUNC		mLayoutFunc;
	float						mResizeLimitWidth,
								mResizeLimitHeight;

	// When true, display the whole sprite area.
	const bool					mDebugShowFrame;

	ci::gl::Texture				mTexture;

#ifdef TEXT_RENDER_ASYNC
	std::shared_ptr<RenderTextShared>
		mShared;
	RenderTextClient			mRenderClient;
	ci::gl::Texture				mTestTexture;
#endif

	// Initialization
public:
	static void					installAsServer(ds::BlobRegistry&);
	static void					installAsClient(ds::BlobRegistry&);
};

} // namespace ui
} // namespace ds

#endif//DS_UI_TEXT_H
