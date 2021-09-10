package kr.ac.kpu.game.s2016182019.termproject.game.object;

import android.graphics.Canvas;
import android.view.MotionEvent;

import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.game.UI.UIManager;

public class Player implements GameObject {

    private final UIManager playerUI;
    public int maxHp = 100;
    public int hp = 100;

    public Enemy enemy;

    public int manaRed = 0;
    public int manaGreen = 0;
    public int manaBlue = 0;
    public int manaBlack = 0;
    public int manaWhite = 0;

    public int attack = 0;
    public int defence = 0;

    public int score = 0;

    public int fire_orb = 0;
    public int rainbow_orb = 0;
    public int spell_damage = 0;
    public int chaos_orb = 0;
    public int thorn = 0;


    public Player() {
        playerUI = new UIManager();
        playerUI.setSkill(1,0);
        playerUI.setSkill(2,1);
        playerUI.setSkill(3,2);
        playerUI.setSkill(4,3);
    }


    public void initialize() {
        manaRed = 5 * fire_orb + rainbow_orb;
        manaGreen = rainbow_orb;
        manaBlue = rainbow_orb;
        manaBlack = rainbow_orb;
        manaWhite = rainbow_orb;

    }
    public void heal() {
        hp += Math.floor(maxHp / 10);
        hp = Math.min(hp,maxHp);
    }

    public boolean useMana(int red, int green, int blue, int black, int white)
    {
        if (manaRed >= red && manaGreen >= green && manaBlue >= blue && manaBlack >= black && manaWhite >= white) {
            manaRed -= red;
            manaGreen -= green;
            manaBlue -= blue;
            manaBlack -= black;
            manaWhite -= white;
            return true;
        }
        return false;
    }

    @Override
    public void update() {
        playerUI.update();
    }

    @Override
    public void draw(Canvas canvas) {
        playerUI.draw(canvas);
    }

    public boolean onTouchEvent(MotionEvent event) {
        return playerUI.onTouchEvent(event);
    }
}
