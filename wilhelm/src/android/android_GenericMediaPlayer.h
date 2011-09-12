/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ANDROID_GENERICMEDIAPLAYER_H__
#define __ANDROID_GENERICMEDIAPLAYER_H__

#include "android_GenericPlayer.h"

#include <binder/IServiceManager.h>
#include <surfaceflinger/Surface.h>
#include <gui/ISurfaceTexture.h>


//--------------------------------------------------------------------------------------------------
namespace android {

class GenericMediaPlayer;
class MediaPlayerNotificationClient : public BnMediaPlayerClient
{
public:
    MediaPlayerNotificationClient(GenericMediaPlayer* gmp);
    virtual ~MediaPlayerNotificationClient();

    // IMediaPlayerClient implementation
    virtual void notify(int msg, int ext1, int ext2, const Parcel *obj);

    // Call before enqueuing a prepare event
    void beforePrepare();

    // Call after enqueueing the prepare event; returns true if the prepare
    // completed successfully, or false if it completed unsuccessfully
    bool blockUntilPlayerPrepared();

private:
    const wp<GenericMediaPlayer> mGenericMediaPlayer;
    Mutex mLock;                        // protects mPlayerPrepared
    Condition mPlayerPreparedCondition; // signalled when mPlayerPrepared is changed
    enum {
        PREPARE_NOT_STARTED,
        PREPARE_IN_PROGRESS,
        PREPARE_COMPLETED_SUCCESSFULLY,
        PREPARE_COMPLETED_UNSUCCESSFULLY
    } mPlayerPrepared;
};


//--------------------------------------------------------------------------------------------------
class GenericMediaPlayer : public GenericPlayer
{
public:

    GenericMediaPlayer(const AudioPlayback_Parameters* params, bool hasVideo);
    virtual ~GenericMediaPlayer();

    virtual void preDestroy();

    // overridden from GenericPlayer
    virtual void getPositionMsec(int* msec); // ANDROID_UNKNOWN_TIME if unknown

    virtual void setVideoSurface(const sp<Surface> &surface);
    virtual void setVideoSurfaceTexture(const sp<ISurfaceTexture> &surfaceTexture);

protected:
    friend class MediaPlayerNotificationClient;

    // Async event handlers (called from GenericPlayer's event loop)
    virtual void onPrepare();
    virtual void onPlay();
    virtual void onPause();
    virtual void onSeek(const sp<AMessage> &msg);
    virtual void onLoop(const sp<AMessage> &msg);
    virtual void onSeekComplete();
    virtual void onVolumeUpdate();
    virtual void onBufferingUpdate(const sp<AMessage> &msg);
    virtual void onAttachAuxEffect(const sp<AMessage> &msg);
    virtual void onSetAuxEffectSendLevel(const sp<AMessage> &msg);

    const bool mHasVideo;   // const allows MediaPlayerNotificationClient::notify to safely access
    int32_t mSeekTimeMsec;

    // at most one of mVideoSurface and mVideoSurfaceTexture is non-NULL
    sp<Surface> mVideoSurface;
    sp<ISurfaceTexture> mVideoSurfaceTexture;

    // only safe to access from within Realize and looper
    sp<IMediaPlayer> mPlayer;
    // Receives Android MediaPlayer events from mPlayer
    const sp<MediaPlayerNotificationClient> mPlayerClient;

    // Return a reference to the media player service, or LOGE and return NULL after retries fail
    static const sp<IMediaPlayerService> getMediaPlayerService() {
        return IMediaDeathNotifier::getMediaPlayerService();
    }

private:
    DISALLOW_EVIL_CONSTRUCTORS(GenericMediaPlayer);
    void afterMediaPlayerPreparedSuccessfully();

protected:  // FIXME temporary
    Mutex mPreparedPlayerLock;          // protects mPreparedPlayer
    sp<IMediaPlayer> mPreparedPlayer;   // non-NULL if MediaPlayer exists and prepared, write once
private:
    void getPreparedPlayer(sp<IMediaPlayer> &preparedPlayer);   // safely read mPreparedPlayer

};

} // namespace android

// is the specified URI a known distant protocol?
bool isDistantProtocol(const char *uri);

#endif /* __ANDROID_GENERICMEDIAPLAYER_H__ */
