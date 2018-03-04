// $Id: PostProcessor.hh 12694 2012-07-06 09:20:35Z m9710797 $

#ifndef POSTPROCESSOR_HH
#define POSTPROCESSOR_HH

#include "FrameSource.hh"
#include "VideoLayer.hh"
#include "EmuTime.hh"
#include <memory>

namespace openmsx {

class Display;
class RenderSettings;
class RawFrame;
class DeinterlacedFrame;
class DoubledFrame;
class SuperImposedFrame;
class AviRecorder;
class CliComm;

/** Abstract base class for post processors.
  * A post processor builds the frame that is displayed from the MSX frame,
  * while applying effects such as scalers, noise etc.
  * TODO: With some refactoring, it would be possible to move much or even all
  *       of the post processing code here instead of in the subclasses.
  */
class PostProcessor : public VideoLayer
{
public:
	virtual ~PostProcessor();

	// Layer interface:
	virtual void paint(OutputSurface& output) = 0;
	virtual string_ref getLayerName() const;

	/** Sets up the "abcdFrame" variables for a new frame.
	  * TODO: The point of passing the finished frame in and the new workFrame
	  *       out is to be able to split off the scaler application as a
	  *       separate class.
	  * @param finishedFrame Frame that has just become available.
	  * @param field Specifies what role (if any) the new frame plays in
	  *   interlacing.
	  * @param time The moment in time the frame becomes available. Used to
	  *             calculate the framerate for recording (depends on
	  *             PAL/NTSC, frameskip).
	  * @return RawFrame object that can be used for building the next frame.
	  */
	virtual std::auto_ptr<RawFrame> rotateFrames(
		std::auto_ptr<RawFrame> finishedFrame, FrameSource::FieldType field,
		EmuTime::param time);

	/** Set the Video frame on which to superimpose the 'normal' output of
	  * this PostProcessor. Superimpose is done (preferably) after the
	  * normal output is scaled. IOW the video frame is (preferably) left
	  * unchanged, though exceptions are e.g. scalers that render
	  * scanlines, those are preferably also visible in the video frame.
	  */
	void setSuperimposeVideoFrame(const RawFrame* videoSource);

	/** Set the VDP frame on which to superimpose the 'normal' output of
	  * this PostProcessor. This is similar to the method above, except
	  * that now the superimposing is done before scaling. IOW both frames
	  * get scaled.
	  */
	void setSuperimposeVdpFrame(const FrameSource* vdpSource);

	/** Start/stop recording.
	  * @param recorder Finished frames should be pushed to this
	  *                 AviRecorder. Can also be a NULL pointer, meaning
	  *                 recording is stopped.
	  */
	void setRecorder(AviRecorder* recorder);

	/** Is recording active.
	  * ATM used to keep frameskip constant during recording.
	  */
	bool isRecording() const;

	/** Get the number of bits per pixel for the pixels in these frames.
	  * @return Possible values are 15, 16 or 32
	  */
	unsigned getBpp() const;

	/** Get the frame that would be displayed. E.g. so that it can be
	  * superimposed over the output of another PostProcessor, see
	  * setSuperimposeVdpFrame().
	  */
	FrameSource* getPaintFrame() const { return paintFrame; }

	// VideoLayer
	virtual void takeRawScreenShot(unsigned height, const std::string& filename);


	CliComm& getCliComm();

protected:
	/** Returns the maximum width for lines [y..y+step).
	  */
	static unsigned getLineWidth(FrameSource* frame, unsigned y, unsigned step);

	PostProcessor(
		MSXMotherBoard& motherBoard, Display& display,
		OutputSurface& screen, VideoSource videoSource,
		unsigned maxWidth, unsigned height);

	/** Render settings */
	RenderSettings& renderSettings;

	/** The surface which is visible to the user. */
	OutputSurface& screen;

	/** The last finished frame, ready to be displayed. */
	std::auto_ptr<RawFrame> currFrame;

	/** The frame before currFrame, ready to be displayed. */
	std::auto_ptr<RawFrame> prevFrame;

	/** Combined currFrame and prevFrame. */
	std::auto_ptr<DeinterlacedFrame> deinterlacedFrame;

	/** Each line of currFrame twice, to get double vertical resolution. */
	std::auto_ptr<DoubledFrame> interlacedFrame;

	/** Result of superimposing 2 frames. */
	std::auto_ptr<SuperImposedFrame> superImposedFrame;

	/** Represents a frame as it should be displayed.
	  * This can be simply a RawFrame or two RawFrames combined in a
	  * DeinterlacedFrame or DoubledFrame.
	  */
	FrameSource* paintFrame;

	/** Video recorder, NULL when not recording. */
	AviRecorder* recorder;

	/** Video frame on which to superimpose the (VDP) output.
	  * NULL when not superimposing. */
	const RawFrame* superImposeVideoFrame;
	const FrameSource* superImposeVdpFrame;

private:
	void getScaledFrame(unsigned height, const void** lines);

	Display& display;
};

} // namespace openmsx

#endif // POSTPROCESSOR_HH
