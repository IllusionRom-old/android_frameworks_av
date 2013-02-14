/*
 * Copyright 2013, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MEDIA_MUXER_H_
#define MEDIA_MUXER_H_

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/threads.h>

namespace android {

struct ABuffer;
struct AMessage;
struct MediaAdapter;
struct MediaBuffer;
struct MediaSource;
struct MetaData;
struct MPEG4Writer;

// MediaMuxer is used to mux multiple tracks into a video. Currently, we only
// support a mp4 file as the output.
// The expected calling order of the functions is:
// Constructor -> addTrack+ -> start -> writeSampleData+ -> stop
// If muxing operation need to be cancelled, the app is responsible for
// deleting the output file after stop.
struct MediaMuxer : public RefBase {
public:
    // Construct the muxer with the output file path.
    MediaMuxer(const char* pathOut);
    // Construct the muxer with the file descriptor. Note that the MediaMuxer
    // will close this file at stop().
    MediaMuxer(int fd);

    virtual ~MediaMuxer();

    /**
     * Add a track with its format information. This should be
     * called before start().
     * @param format the track's format.
     * @return the track's index or negative number if error.
     */
    ssize_t addTrack(const sp<AMessage> &format);

    /**
     * Start muxing. Make sure all the tracks have been added before
     * calling this.
     */
    status_t start();

    /**
     * Stop muxing.
     * This method is a blocking call. Depending on how
     * much data is bufferred internally, the time needed for stopping
     * the muxer may be time consuming. UI thread is
     * not recommended for launching this call.
     */
    status_t stop();

    /**
     * Send a sample buffer for muxing.
     * The buffer can be reused once this method returns. Typically,
     * this function won't be blocked for very long, and thus there
     * is no need to use a separate thread calling this method to
     * push a buffer.
     * @param buffer the incoming sample buffer.
     * @param trackIndex the buffer's track index number.
     * @param timeUs the buffer's time stamp.
     * @param flags the only supported flag for now is
     *              MediaCodec::BUFFER_FLAG_SYNCFRAME.
     * @return OK if no error.
     */
    status_t writeSampleData(const sp<ABuffer> &buffer, size_t trackIndex,
                             int64_t timeUs, uint32_t flags) ;

private:
    sp<MPEG4Writer> mWriter;
    Vector< sp<MediaAdapter> > mTrackList;  // Each track has its MediaAdapter.

    Mutex mMuxerLock;

    enum State {
        INITED,
        STARTED,
        STOPPED
    };
    State mState;

    DISALLOW_EVIL_CONSTRUCTORS(MediaMuxer);
};

}  // namespace android

#endif  // MEDIA_MUXER_H_
