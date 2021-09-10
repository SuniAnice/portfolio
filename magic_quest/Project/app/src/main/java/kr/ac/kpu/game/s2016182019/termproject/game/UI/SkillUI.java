package kr.ac.kpu.game.s2016182019.termproject.game.UI;

import android.graphics.Canvas;
import android.graphics.RectF;

import java.util.Random;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.BoxCollidable;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.Sound;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.BaseGame;
import kr.ac.kpu.game.s2016182019.termproject.game.Scene.MainScene;
import kr.ac.kpu.game.s2016182019.termproject.game.object.Block;

public class SkillUI implements GameObject, BoxCollidable {
    private final int x;
    private final int y;
    private int skillIndex;

    public int[] skills = {
            R.mipmap.skill1, R.mipmap.skill2, R.mipmap.skill3, R.mipmap.skill4
    };
    private GameBitmap bitmap;

    public SkillUI(int x, int y) {
        this.x = x;
        this.y = y;
    }

    @Override
    public void update() {

    }

    @Override
    public void draw(Canvas canvas) {
        if (bitmap != null)
            bitmap.draw(canvas, x, y);
    }

    public void setSkill(int index) {
        skillIndex = index;
        this.bitmap = new GameBitmap(skills[index]);
    }

    @Override
    public void getBoundingRect(RectF rect) {
        bitmap.getBoundingRect(x, y, rect);
    }

    public void useSkill() {
        BaseGame game = BaseGame.get();

        switch (skillIndex) {
            case 0:
                if (MainScene.scene.player.useMana(0,5,5,5,5))
                {
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            if (MainScene.scene.board.blocks[i][j] != null) {
                                if (MainScene.scene.board.blocks[i][j].type == Block.blockType.Blue || MainScene.scene.board.blocks[i][j].type == Block.blockType.Green)
                                {
                                    MainScene.scene.board.blocks[i][j].change(Block.blockType.Red);
                                }
                            }
                        }
                    }

                    Sound.play(R.raw.magic);
                    MainScene.scene.enemy.hp -= MainScene.scene.player.spell_damage;
                }

                break;
            case 1:
                if (MainScene.scene.player.useMana(10,0,0,0,0))
                {
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            if (MainScene.scene.board.blocks[i][j] != null) {
                                if (MainScene.scene.board.blocks[i][j].type == Block.blockType.Red)
                                {
                                    MainScene.scene.board.blocks[i][j].change(Block.blockType.Sword);
                                }
                            }
                        }
                    }
                    Sound.play(R.raw.magic);
                    MainScene.scene.enemy.hp -= MainScene.scene.player.spell_damage;

                }
                break;
            case 2:
                if (MainScene.scene.player.useMana(0,0,0,0,10))
                {
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            if (MainScene.scene.board.blocks[i][j] != null) {
                                if (MainScene.scene.board.blocks[i][j].type == Block.blockType.White)
                                {
                                    MainScene.scene.board.blocks[i][j].change(Block.blockType.Shield);
                                }
                            }
                        }
                    }
                    Sound.play(R.raw.magic);
                    MainScene.scene.enemy.hp -= MainScene.scene.player.spell_damage;

                }
                break;
            case 3:
                if (MainScene.scene.player.useMana(0,0,0,10,0))
                {
                    Random r = new Random();
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            if (MainScene.scene.board.blocks[i][j] != null) {
                                MainScene.scene.board.blocks[i][j].change(Block.blockType.values()[r.nextInt(7)]);
                            }
                        }
                    }
                    for (int i = 0 ; i < MainScene.scene.player.chaos_orb * 5; i++) {
                        int x = r.nextInt(8);
                        int y = r.nextInt(8);
                        MainScene.scene.board.blocks[x][y].boom = true;
                    }
                    Sound.play(R.raw.magic);
                    MainScene.scene.enemy.hp -= MainScene.scene.player.spell_damage;

                }
                break;
        }
    }
}
