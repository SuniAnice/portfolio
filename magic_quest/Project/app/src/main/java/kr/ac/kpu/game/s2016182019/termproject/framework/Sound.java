package kr.ac.kpu.game.s2016182019.termproject.framework;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.os.Build;
import android.util.Log;

import java.util.HashMap;

import kr.ac.kpu.game.s2016182019.termproject.R;


public class Sound {
    private static final String TAG = kr.ac.kpu.game.s2016182019.termproject.framework.Sound.class.getSimpleName();
    private static SoundPool soundPool;

    private static final int[] SOUND_IDS = {
            R.raw.mana, R.raw.magic, R.raw.damage, R.raw.attack
    };
    private static HashMap<Integer, Integer> soundIdMap = new HashMap<Integer, Integer>();
    private static int bgmID = 0;
    public static MediaPlayer mp;

    public static void init(Context context) {
        AudioAttributes audioAttributes;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            audioAttributes = new AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_GAME)
                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .build();
            kr.ac.kpu.game.s2016182019.termproject.framework.Sound.soundPool = new SoundPool.Builder()
                    .setAudioAttributes(audioAttributes)
                    .setMaxStreams(5)
                    .build();
        } else {
            kr.ac.kpu.game.s2016182019.termproject.framework.Sound.soundPool = new SoundPool(5, AudioManager.STREAM_MUSIC,0);
        }

        for (int resId : SOUND_IDS) {
            int soundId = soundPool.load(context, resId, 1);
            soundIdMap.put(resId, soundId);
        }
        mp = MediaPlayer.create(context, R.raw.battle);
        mp.setLooping(true);
        mp.start();
    }

    public static void stop() {
        mp.stop();
    }



    public static int play(int resId) {
        Log.d(TAG, "play : " + resId);
        int soundId = soundIdMap.get(resId);
        int streamId = soundPool.play(soundId, 1f, 1f, 1, 0, 1f);
        return streamId;
    }
}
