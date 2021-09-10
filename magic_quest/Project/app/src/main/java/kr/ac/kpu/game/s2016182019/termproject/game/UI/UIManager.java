package kr.ac.kpu.game.s2016182019.termproject.game.UI;

import android.graphics.Canvas;
import android.view.MotionEvent;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.BaseGame;
import kr.ac.kpu.game.s2016182019.termproject.game.Scene.MainScene;
import kr.ac.kpu.game.s2016182019.termproject.utils.CollisionHelper;

public class UIManager implements GameObject {
    private final GameBitmap charBitmap;
    private final Text whiteText;
    private final Text blackText;
    private final GameBitmap manaUiBitmap;
    private final Text healthText;
    private final Text redText;
    private final Text greenText;
    private final Text blueText;
    private SkillUI[] skillSlots = new SkillUI[4];




    public UIManager() {
        charBitmap = new GameBitmap(R.mipmap.mage);
        manaUiBitmap = new GameBitmap(R.mipmap.manaui);
        healthText = new Text(250, 335);
        redText = new Text(450, 335);
        greenText = new Text(250, 415);
        blueText = new Text(450, 415);
        blackText = new Text(450, 500);
        whiteText = new Text(250, 500);
        skillSlots[0] = new SkillUI(270,640);
        skillSlots[1] = new SkillUI(270,750);
        skillSlots[2] = new SkillUI(270,860);
        skillSlots[3] = new SkillUI(270,970);
    }

    public void setSkill(int slot, int index) {
        switch (slot) {
            case 1:
                skillSlots[0].setSkill(index);
                break;
            case 2:
                skillSlots[1].setSkill(index);
                break;
            case 3:
                skillSlots[2].setSkill(index);
                break;
            case 4:
                skillSlots[3].setSkill(index);
                break;
        }
    }

    @Override
    public void update() {
        BaseGame game = BaseGame.get();
        healthText.setNum(MainScene.scene.player.hp);
        redText.setNum(MainScene.scene.player.manaRed);
        greenText.setNum(MainScene.scene.player.manaGreen);
        blueText.setNum(MainScene.scene.player.manaBlue);
        blackText.setNum(MainScene.scene.player.manaBlack);
        whiteText.setNum(MainScene.scene.player.manaWhite);
    }

    @Override
    public void draw(Canvas canvas) {
        charBitmap.draw(canvas, 260, 150, 1.1f);
        manaUiBitmap.draw(canvas, 270, 450, 0.58f);
        healthText.draw(canvas);
        redText.draw(canvas);
        greenText.draw(canvas);
        blueText.draw(canvas);
        blackText.draw(canvas);
        whiteText.draw(canvas);
        for (SkillUI slot : skillSlots) {
            slot.draw(canvas);
        }
    }

    public boolean onTouchEvent(MotionEvent event) {
        float x = event.getX();
        float y = event.getY();

        for (int i = 0; i < 4 ; i++) {
            if (CollisionHelper.collides(skillSlots[i], x, y)) {
                skillSlots[i].useSkill();
            }
        }

        return false;
    }
}
