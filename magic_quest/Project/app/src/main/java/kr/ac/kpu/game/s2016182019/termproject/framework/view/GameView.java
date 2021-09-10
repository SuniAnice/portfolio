package kr.ac.kpu.game.s2016182019.termproject.framework.view;

import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Choreographer;
import android.view.MotionEvent;
import android.view.View;

import androidx.annotation.Nullable;

import kr.ac.kpu.game.s2016182019.termproject.framework.Sound;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.BaseGame;

public class GameView extends View {
    private static final String TAG = GameView.class.getSimpleName();

    public static float MULTIPLIER = 2.7f;
    private boolean running;

    private long lastFrame;
    public static GameView view;



    public GameView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        GameView.view = this;
        kr.ac.kpu.game.s2016182019.termproject.framework.Sound.init(context);
        running = true;
        //startUpdating();
    }


    private void update() {
//        update();
        BaseGame game = BaseGame.get();
        game.update();

//        draw();
        invalidate();
    }
    private void requestCallback(){
        if (!running) {
            return;
        }
        Choreographer.getInstance().postFrameCallback(new Choreographer.FrameCallback() {
            @Override
            public void doFrame(long time) {
                if (lastFrame == 0){
                    lastFrame = time;
                }
                BaseGame game = BaseGame.get();
                game.frameTime = (float) (time - lastFrame) / 1_000_000_000;
                update();
                lastFrame = time;
                requestCallback();
            }
        });


    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        Log.d(TAG, "onSize: " + w + "," + h);
        BaseGame game = BaseGame.get();
        //game.initResources();
        boolean justInitialized = game.initResources();
        if (justInitialized) {
            requestCallback();
        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        BaseGame game = BaseGame.get();
        game.draw(canvas);

    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        BaseGame game = BaseGame.get();
        return game.onTouchEvent(event);

    }

    public void pauseGame() {
        running = false;
        Sound.mp.pause();
    }

    public void resumeGame() {
        if (!running) {
            running = true;
            lastFrame = 0;
            Sound.mp.start();
            requestCallback();
        }
    }

    public void finishActivity() {
        Activity activity = getActivity();
        Sound.stop();
        activity.finish();
    }

    private Activity getActivity() {
        Context context = getContext();
        while (context instanceof ContextWrapper) {
            if (context instanceof Activity) {
                return (Activity) context;
            }
            context = ((ContextWrapper) context).getBaseContext();
        }
        return null;
    }

    public boolean handleBackKey() {
        return BaseGame.get().handleBackKey();
    }
}
