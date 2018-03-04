// $Id: OSDText.cc 12674 2012-06-25 18:13:20Z m9710797 $

#include "OSDText.hh"
#include "TTFFont.hh"
#include "SDLImage.hh"
#include "OutputRectangle.hh"
#include "CommandException.hh"
#include "FileContext.hh"
#include "FileOperations.hh"
#include "TclObject.hh"
#include "StringOp.hh"
#include "utf8_core.hh"
#include "unreachable.hh"
#include "components.hh"
#include <cassert>
#if COMPONENT_GL
#include "GLImage.hh"
#endif

using std::string;
using std::vector;

namespace openmsx {

OSDText::OSDText(const OSDGUI& gui, const string& name)
	: OSDImageBasedWidget(gui, name)
	, fontfile("skins/Vera.ttf.gz")
	, size(12)
	, wrapMode(NONE), wrapw(0.0), wraprelw(1.0)
{
}

OSDText::~OSDText()
{
}

void OSDText::getProperties(std::set<string>& result) const
{
	result.insert("-text");
	result.insert("-font");
	result.insert("-size");
	result.insert("-wrap");
	result.insert("-wrapw");
	result.insert("-wraprelw");
	result.insert("-query-size");
	OSDImageBasedWidget::getProperties(result);
}

void OSDText::setProperty(string_ref name, const TclObject& value)
{
	if (name == "-text") {
		string_ref val = value.getString();
		if (text != val) {
			text = val.str();
			// note: don't invalidate font (don't reopen font file)
			OSDImageBasedWidget::invalidateLocal();
			invalidateChildren();
		}
	} else if (name == "-font") {
		string val = value.getString().str();
		if (fontfile != val) {
			SystemFileContext context;
			string file = context.resolve(val);
			if (!FileOperations::isRegularFile(file)) {
				throw CommandException("Not a valid font file: " + val);
			}
			fontfile = val;
			invalidateRecursive();
		}
	} else if (name == "-size") {
		int size2 = value.getInt();
		if (size != size2) {
			size = size2;
			invalidateRecursive();
		}
	} else if (name == "-wrap") {
		string_ref val = value.getString();
		WrapMode wrapMode2;
		if (val == "none") {
			wrapMode2 = NONE;
		} else if (val == "word") {
			wrapMode2 = WORD;
		} else if (val == "char") {
			wrapMode2 = CHAR;
		} else {
			throw CommandException("Not a valid value for -wrap, "
				"expected one of 'none word char', but got '" +
				val + "'.");
		}
		if (wrapMode != wrapMode2) {
			wrapMode = wrapMode2;
			invalidateRecursive();
		}
	} else if (name == "-wrapw") {
		double wrapw2 = value.getDouble();
		if (wrapw != wrapw2) {
			wrapw = wrapw2;
			invalidateRecursive();
		}
	} else if (name == "-wraprelw") {
		double wraprelw2 = value.getDouble();
		if (wraprelw != wraprelw2) {
			wraprelw = wraprelw2;
			invalidateRecursive();
		}
	} else if (name == "-query-size") {
		throw CommandException("-query-size property is readonly");
	} else {
		OSDImageBasedWidget::setProperty(name, value);
	}
}

void OSDText::getProperty(string_ref name, TclObject& result) const
{
	if (name == "-text") {
		result.setString(text);
	} else if (name == "-font") {
		result.setString(fontfile);
	} else if (name == "-size") {
		result.setInt(size);
	} else if (name == "-wrap") {
		string wrapString;
		switch (wrapMode) {
			case NONE: wrapString = "none"; break;
			case WORD: wrapString = "word"; break;
			case CHAR: wrapString = "char"; break;
			default: UNREACHABLE;
		}
		result.setString(wrapString);
	} else if (name == "-wrapw") {
		result.setDouble(wrapw);
	} else if (name == "-wraprelw") {
		result.setDouble(wraprelw);
	} else if (name == "-query-size") {
		double outX, outY;
		getRenderedSize(outX, outY);
		result.addListElement(outX);
		result.addListElement(outY);
	} else {
		OSDImageBasedWidget::getProperty(name, result);
	}
}

void OSDText::invalidateLocal()
{
	font.reset();
	OSDImageBasedWidget::invalidateLocal();
}


string_ref OSDText::getType() const
{
	return "text";
}

void OSDText::getWidthHeight(const OutputRectangle& /*output*/,
                             double& width, double& height) const
{
	if (image.get()) {
		width  = image->getWidth();
		height = image->getHeight();
	} else {
		// we don't know the dimensions, must be because of an error
		assert(hasError());
		width  = 0;
		height = 0;
	}
}

byte OSDText::getFadedAlpha() const
{
	return byte((getRGBA(0) & 0xff) * getRecursiveFadeValue());
}

template <typename IMAGE> BaseImage* OSDText::create(OutputRectangle& output)
{
	if (text.empty()) {
		return new IMAGE(0, 0, unsigned(0));
	}
	int scale = getScaleFactor(output);
	if (!font.get()) {
		try {
			SystemFileContext context;
			string file = context.resolve(fontfile);
			int ptSize = size * scale;
			font.reset(new TTFFont(file, ptSize));
		} catch (MSXException& e) {
			throw MSXException("Couldn't open font: " + e.getMessage());
		}
	}
	try {
		double pWidth, pHeight;
		getParent()->getWidthHeight(output, pWidth, pHeight);
		int maxWidth = int(wrapw * scale + wraprelw * pWidth + 0.5);
		// Width can't be negative, if it is make it zero instead.
		// This will put each character on a different line.
		maxWidth = std::max(0, maxWidth);

		// TODO gradient???
		unsigned rgba = getRGBA(0);
		string wrappedText;
		if (wrapMode == NONE) {
			wrappedText = text; // don't wrap
		} else if (wrapMode == WORD) {
			wrappedText = getWordWrappedText(text, maxWidth);
		} else if (wrapMode == CHAR) {
			wrappedText = getCharWrappedText(text, maxWidth);
		} else {
			UNREACHABLE;
		}
		// An alternative is to pass vector<string> to TTFFont::render().
		// That way we can avoid StringOp::join() (in the wrap functions)
		// followed by // StringOp::split() (in TTFFont::render()).
		SDLSurfacePtr surface(font->render(wrappedText,
			(rgba >> 24) & 0xff, (rgba >> 16) & 0xff, (rgba >> 8) & 0xff));
		if (surface.get()) {
			return new IMAGE(surface);
		} else {
			return new IMAGE(0, 0, unsigned(0));
		}
	} catch (MSXException& e) {
		throw MSXException("Couldn't render text: " + e.getMessage());
	}
}


// Search for a position strictly between min and max which also points to the
// start of a (possibly multi-byte) utf8-character. If no such position exits,
// this function returns 'min'.
static unsigned findCharSplitPoint(const string& line, unsigned min, unsigned max)
{
	unsigned pos = (min + max) / 2;
	const char* beginIt = line.data();
	const char* posIt = beginIt + pos;

	const char* fwdIt = utf8::sync_forward(posIt);
	const char* maxIt = beginIt + max;
	assert(fwdIt <= maxIt);
	if (fwdIt != maxIt) {
		return fwdIt - beginIt;
	}

	const char* bwdIt = utf8::sync_backward(posIt);
	const char* minIt = beginIt + min;
	assert(minIt <= bwdIt); (void)minIt;
	return bwdIt - beginIt;
}

// Search for a position that's strictly between min and max and which points
// to a character directly following a delimiter character. if no such position
// exits, this function returns 'min'.
// This function works correctly with multi-byte utf8-encoding as long as
// all delimiter characters are single byte chars.
static unsigned findWordSplitPoint(string_ref line, unsigned min, unsigned max)
{
	static const char* const delimiters = " -/";

	// initial guess for a good position
	assert(min < max);
	unsigned pos = (min + max) / 2;
	if (pos == min) {
		// can't reduce further
		return min;
	}

	// try searching backward (this also checks current position)
	assert(pos > min);
	string_ref::size_type pos2 = line.substr(min, pos - min).find_last_of(delimiters);
	if (pos2 != string_ref::npos) {
		pos2 += min + 1;
		assert(min < pos2);
		assert(pos2 <= pos);
		return unsigned(pos2);
	}

	// try searching forward
	string_ref::size_type pos3 = line.substr(pos, max - pos).find_first_of(delimiters);
	if (pos3 != string_ref::npos) {
		pos3 += pos;
		assert(pos3 < max);
		pos3 += 1; // char directly after a delimiter;
		if (pos3 < max) {
			return unsigned(pos3);
		}
	}

	return min;
}

static unsigned takeSingleChar(const string& /*line*/, unsigned /*maxWidth*/)
{
	return 1;
}

template<typename FindSplitPointFunc, typename CantSplitFunc>
unsigned OSDText::split(const string& line, unsigned maxWidth,
                        FindSplitPointFunc findSplitPoint,
                        CantSplitFunc cantSplit,
                        bool removeTrailingSpaces) const
{
	if (line.empty()) {
		// empty line always fits (explicitly handle this because
		// SDL_TTF can't handle empty strings)
		return 0;
	}

	unsigned width, height;
	font->getSize(line, width, height);
	if (width <= maxWidth) {
		// whole line fits
		return unsigned(line.size());
	}

	// binary search till we found the largest initial substring that is
	// not wider than maxWidth
	unsigned min = 0;
	unsigned max = unsigned(line.size());
	// invariant: line.substr(0, min) DOES     fit
	//            line.substr(0, max) DOES NOT fit
	unsigned cur = findSplitPoint(line, min, max);
	if (cur == 0) {
		// Could not find a valid split point, then split on char
		// (this also handles the case of a single too wide char)
		return cantSplit(line, maxWidth);
	}
	while (true) {
		assert(min < cur);
		assert(cur < max);
		string curStr = line.substr(0, cur);
		if (removeTrailingSpaces) {
			StringOp::trimRight(curStr, ' ');
		}
		font->getSize(curStr, width, height);
		if (width <= maxWidth) {
			// still fits, try to enlarge
			unsigned next = findSplitPoint(line, cur, max);
			if (next == cur) {
				return cur;
			}
			min = cur;
			cur = next;
		} else {
			// doesn't fit anymore, try to shrink
			unsigned next = findSplitPoint(line, min, cur);
			if (next == min) {
				if (min == 0) {
					// even the first word does not fit,
					// split on char (see above)
					return cantSplit(line, maxWidth);
				}
				return min;
			}
			max = cur;
			cur = next;
		}
	}
}

unsigned OSDText::splitAtChar(const std::string& line, unsigned maxWidth) const
{
	return split(line, maxWidth, findCharSplitPoint, takeSingleChar, false);
}

struct SplitAtChar {
	SplitAtChar(const OSDText& osdText_) : osdText(osdText_) {}
	unsigned operator()(const string& line, unsigned maxWidth) {
		return osdText.splitAtChar(line, maxWidth);
	}
	const OSDText& osdText;
};
unsigned OSDText::splitAtWord(const std::string& line, unsigned maxWidth) const
{
	return split(line, maxWidth, findWordSplitPoint, SplitAtChar(*this), true);
}

string OSDText::getCharWrappedText(const string& text, unsigned maxWidth) const
{
	vector<string> lines;
	StringOp::split(text, "\n", lines);

	vector<string> wrappedLines;
	for (vector<string>::const_iterator it = lines.begin();
	     it != lines.end(); ++it) {
		string line = *it;
		do {
			unsigned pos = splitAtChar(line, maxWidth);
			wrappedLines.push_back(line.substr(0, pos));
			line = line.substr(pos);
		} while (!line.empty());
	}

	return StringOp::join(wrappedLines, "\n");
}

string OSDText::getWordWrappedText(const string& text, unsigned maxWidth) const
{
	vector<string> lines;
	StringOp::split(text, "\n", lines);

	vector<string> wrappedLines;
	for (vector<string>::const_iterator it = lines.begin();
	     it != lines.end(); ++it) {
		string line = *it;
		do {
			unsigned pos = splitAtWord(line, maxWidth);
			string_ref first = string_ref(line).substr(0, pos);
			StringOp::trimRight(first, ' '); // remove trailing spaces
			wrappedLines.push_back(first.str());
			line = line.substr(pos);
			StringOp::trimLeft(line, " "); // remove leading spaces
		} while (!line.empty());
	}

	return StringOp::join(wrappedLines, "\n");
}

void OSDText::getRenderedSize(double& outX, double& outY) const
{
	SDL_Surface* surface = SDL_GetVideoSurface();
	if (!surface) {
		throw CommandException(
			"Can't query size: no window visible");
	}
	DummyOutputRectangle output(surface->w, surface->h);
	// force creating image (does not yet draw it on screen)
	const_cast<OSDText*>(this)->createImage(output);

	unsigned width = 0;
	unsigned height = 0;
	if (image.get()) {
		width  = image->getWidth();
		height = image->getHeight();
	}

	double scale = getScaleFactor(output);
	outX = width  / scale;
	outY = height / scale;
}

BaseImage* OSDText::createSDL(OutputRectangle& output)
{
	return create<SDLImage>(output);
}

BaseImage* OSDText::createGL(OutputRectangle& output)
{
#if COMPONENT_GL
	return create<GLImage>(output);
#else
	(void)&output;
	return NULL;
#endif
}

} // namespace openmsx
