package kr.ac.kpu.game.s2016182019.termproject.game.object;

import android.graphics.Canvas;
import android.graphics.RectF;
import android.view.MotionEvent;

import java.util.Random;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.AnimationGameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.Sound;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.BaseGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;
import kr.ac.kpu.game.s2016182019.termproject.game.Scene.MainScene;
import kr.ac.kpu.game.s2016182019.termproject.game.UI.Effector;
import kr.ac.kpu.game.s2016182019.termproject.utils.CollisionHelper;

import static kr.ac.kpu.game.s2016182019.termproject.utils.SwapHelper.swap;

public class Board implements GameObject {
    static Board instance;
    private final int x;
    private final int y;
    private final GameBitmap bitmap;
    private final float gridX;
    private final float gridY;
    private final float offsetX;
    private final float offsetY;
    private Block selected = null;

    public boolean myTurn = true;
    public boolean canMove = true;
    public int turn = 5;
    private int movingBlocks = 0;

    public Block blocks[][];
    private int selectX;
    private int selectY;
    private boolean soundflag = false;
    private boolean attackflag = false;

    public static Board get() {
        if (instance == null) {
            instance = new Board();
        }
        return instance;
    }

    public Board() {
        this.x = (int) (GameView.view.getWidth() / 2 / 1.33f);
        this.y = (int) (GameView.view.getHeight() / 2 / 1.33f);
        this.bitmap = new GameBitmap(R.mipmap.board);

        RectF bound = new RectF();

        bitmap.getBoundingRect(x, y, bound);

        gridX = (bound.right - bound.left) / 8;
        gridY = (bound.bottom - bound.top) / 8;
        offsetX = bound.left;
        offsetY = bound.top;

        blocks = new Block[8][8];

        Random r = new Random();

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                blocks[i][j] = new Block((int)((gridX * (i + 0.5) + offsetX) / 1.33f), (int)((gridY * (j + 0.5) + offsetY)/ 1.33f), r.nextInt(7));
            }
        }
    }
    @Override
    public void update() {
        BaseGame game = BaseGame.get();
        movingBlocks = 0;
        Random r = new Random();

        if (canMove) {
            // 연속된 블럭 체크
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (blocks[i][j] != null)
                    {
                        Block.blockType tIndex = blocks[i][j].type;
                        // 가로
                        {
                            int count = 1;
                            while (i + count < 8) {
                                if (blocks[i + count][j].type == tIndex) {
                                    count++;
                                } else {
                                    break;
                                }
                            }
                            if (count >= 3) {
                                for (int k = 0; k < count; k++) {
                                    blocks[i + k][j].boom = true;
                                }
                            }
                        }
                        // 세로
                        {
                            int count = 1;
                            while (j + count < 8) {
                                if (blocks[i][j + count].type == tIndex){
                                    count++;
                                } else {
                                    break;
                                }
                            }
                            if (count >= 3) {
                                for (int k = 0; k < count; k++) {
                                    blocks[i][j + k].boom = true;
                                }
                            }
                        }
                        // 대각선1
                        {
                            int count = 1;
                            while (i + count < 8 && j + count < 8) {
                                if (blocks[i + count][j + count].type == tIndex){
                                    count++;
                                } else {
                                    break;
                                }
                            }
                            if (count >= 3) {
                                for (int k = 0; k < count; k++) {
                                    blocks[i + k][j + k].boom = true;
                                }
                            }
                        }
                        // 대각선2
                        {
                            int count = 1;
                            while (i - count >= 0 && j + count < 8) {
                                if (blocks[i - count][j + count].type == tIndex){
                                    count++;
                                } else {
                                    break;
                                }
                            }
                            if (count >= 3) {
                                for (int k = 0; k < count; k++) {
                                    blocks[i - k][j + k].boom = true;
                                }
                            }
                        }
                    }

                }
            }
        }

        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (blocks[i][j] != null) {
                    if (blocks[i][j].boom) {
                        switch (blocks[i][j].type){
                            case Red:
                                MainScene.scene.player.manaRed++;
                                break;
                            case Green:
                                MainScene.scene.player.manaGreen++;
                                break;
                            case Blue:
                                MainScene.scene.player.manaBlue++;
                                break;
                            case Black:
                                MainScene.scene.player.manaBlack++;
                                break;
                            case White:
                                MainScene.scene.player.manaWhite++;
                                break;
                            case Sword:
                                if (MainScene.scene.enemy.hp > 0)
                                    MainScene.scene.enemy.hp--;
                                attackflag = true;
                                break;
                            case Shield:
                                if (MainScene.scene.enemy.currAttack > 0)
                                    MainScene.scene.enemy.currAttack--;
                                break;
                        }
                        soundflag = true;
                        Effector e = new Effector(new AnimationGameBitmap(R.mipmap.explosion,8,8),blocks[i][j].x, blocks[i][j].y, 0.9f);
                        MainScene.scene.add(MainScene.Layer.effect, e);
                        blocks[i][j] = null;
                        movingBlocks++;
                        continue;
                    }
                    if (j < 7) {
                        if (blocks[i][j + 1] == null) {
                            blocks[i][j].moveto((int)((gridX * (i + 0.5) + offsetX)/ 1.33f), (int)((gridY * (j + 1.5) + offsetY)/1.33f));
                            blocks[i][j + 1] = blocks[i][j];
                            blocks[i][j] = null;
                            movingBlocks++;
                            continue;
                        }
                    }
                    if (blocks[i][j].isMoving) {
                        movingBlocks++;
                    }

                    blocks[i][j].update();
                }
            }
        }

        if (soundflag) {
            Sound.play(R.raw.mana);
            soundflag = false;
        }
        if (attackflag) {
            Effector e = new Effector(new AnimationGameBitmap(R.mipmap.damage, 5, 5),1825,350, 0.8f);
            MainScene.scene.add(MainScene.Layer.effect, e);
            Sound.play(R.raw.attack);
            MainScene.scene.enemy.hp -= MainScene.scene.player.attack;
            attackflag = false;
        }
        for (int i = 0; i< 8;i++){
            if (blocks[i][0] == null){
                blocks[i][0] = new Block((int)((gridX * (i + 0.5) + offsetX)/ 1.33f), (int)((gridY * (-0.5) + offsetY)/1.33f), r.nextInt(7));
                blocks[i][0].moveto((int)((gridX * (i + 0.5) + offsetX)/ 1.33f), (int)((gridY * (0.5) + offsetY)/1.33f));
            }
        }

        // 움직이고 있는 블럭 체크
        if (movingBlocks != 0) {
            canMove = false;
        } else {
            canMove = true;
        }
    }

    @Override
    public void draw(Canvas canvas) {
        bitmap.draw(canvas, x, y);
        for (Block[] temp : blocks) {
            for (Block block : temp) {
                if (block != null)
                    block.draw(canvas);
            }
        }
    }

    public boolean onTouchEvent(MotionEvent event) {
        BaseGame game = BaseGame.get();
        float x = event.getX();
        float y = event.getY();

        if (canMove && myTurn){
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (CollisionHelper.collides(blocks[i][j], x, y))
                    {
                        if (selected != null)
                        {
                            if (selected == blocks[i][j]) {
                                selected.select(false);
                                selected = null;
                            }
                            else if (Math.abs(i - selectX) <= 1 && Math.abs(j - selectY) <= 1) {
                                selected.select(false);
                                swap(selected, blocks[i][j]);
                                Block temp = selected;
                                blocks[selectX][selectY] = blocks[i][j];
                                blocks[i][j] = temp;
                                selected = null;
                                canMove = false;
                                turn--;
                                if (turn == 0) {
                                    turn = MainScene.scene.enemy.turn;
                                    MainScene.scene.enemy.doAttack();
                                }
                            }
                            else
                            {
                                selected.select(false);
                                selected = blocks[i][j];
                                blocks[i][j].select(true);
                                selectX = i;
                                selectY = j;
                            }
                        }
                        else
                        {
                            selected = blocks[i][j];
                            blocks[i][j].select(true);
                            selectX = i;
                            selectY = j;
                        }

                        return true;
                    }
                }
            }
        }


        return false;
    }

}
