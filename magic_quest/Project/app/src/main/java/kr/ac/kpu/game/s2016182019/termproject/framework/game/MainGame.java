package kr.ac.kpu.game.s2016182019.termproject.framework.game;

import kr.ac.kpu.game.s2016182019.termproject.game.Scene.TitleScene;

public class MainGame extends BaseGame {
    private boolean initialized;

    public static MainGame get() {
        return (MainGame) instance;
    }


    @Override
    public boolean initResources() {
        if (initialized) {
            return false;
        }

        push(new TitleScene());

        initialized = true;
        return true;

    }
}
