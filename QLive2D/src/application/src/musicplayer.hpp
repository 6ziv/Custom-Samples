#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QMediaPlayer>
#include <bass.h>
#include <QFile>
#include <optional>
class MusicPlayer
{
public:
    inline static MusicPlayer* GetInstance(){
        return s_instance==nullptr?new MusicPlayer:s_instance;
    }
    inline void setmute(bool mute){
        muted=mute;
        if(music.has_value())
            BASS_ChannelSetAttribute(music.value(), BASS_ATTRIB_VOL, mute?0:1);
    }
    inline bool isMuted(){
        return muted;
    }
    inline bool setMusic(const wchar_t *filename){
        if(filename==nullptr){
            if(music.has_value())
                BASS_StreamFree(music.value());
            BASS_Stop();
            return true;
        }
        HSTREAM hs=BASS_StreamCreateFile(false,filename,0,0,BASS_SAMPLE_LOOP|BASS_UNICODE);
        if(hs==0)return false;
        if(music.has_value())
            BASS_StreamFree(music.value());
        music=hs;
        BASS_ChannelSetAttribute(music.value(), BASS_ATTRIB_VOL, muted?0:1);
        if(!BASS_IsStarted())BASS_Start();
        BASS_ChannelPlay(hs,true);
        return true;
    }
private:
    inline static MusicPlayer *s_instance=nullptr;
    inline MusicPlayer(){
        s_instance=this;
        BASS_Init(-1,44100,BASS_DEVICE_STEREO|BASS_DEVICE_16BITS,NULL,NULL);
    }
    bool muted=false;
    std::optional<HSTREAM> music;

};

#endif // MUSICPLAYER_H
