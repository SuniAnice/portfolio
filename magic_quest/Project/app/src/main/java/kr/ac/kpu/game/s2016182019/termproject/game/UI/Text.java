package kr.ac.kpu.game.s2016182019.termproject.game.UI;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.graphics.RectF;

import kr.ac.kpu.game.s2016182019.termproject.R;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameBitmap;
import kr.ac.kpu.game.s2016182019.termproject.framework.GameObject;
import kr.ac.kpu.game.s2016182019.termproject.framework.game.MainGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;

public class Text implements GameObject {
    private final Bitmap bitmap;
    private final int x;
    private int y;
    private int gy = -1;

    private Rect src = new Rect();
    private RectF dst = new RectF();
    private int num;
    private int displaynum;

    public void setNum(int score) {
        this.num = score;
        this.displaynum = score;
    }

    public void addnum(int amount) {
        this.num += amount;
    }



    public Text(int x, int y){
        bitmap = GameBitmap.load(R.mipmap.number_24x32);
        this.x = (int) (x*1.33f);
        this.y = (int) (y*1.33f);
    }

    public Text(int x, int y, int gy){
        bitmap = GameBitmap.load(R.mipmap.number_24x32);
        this.x = (int) (x*1.33f);
        this.y = (int) (y*1.33f);
        this.gy = gy;
    }

    @Override
    public void update() {
        if (displaynum < num) {
            displaynum++;
        }
        MainGame game = MainGame.get();
        if (gy > 0) {
            y += game.frameTime * 500;
            gy -= game.frameTime * 500;
        }
    }

    @Override
    public void draw(Canvas canvas) {
        int value = this.displaynum;
        int nw = bitmap.getWidth() / 10;
        int nh = bitmap.getHeight();
        int dw = (int) (nw * GameView.MULTIPLIER);
        int dh = (int) (nh * GameView.MULTIPLIER);
        int tx = x;
        do {
            int digit = value % 10;
            src.set(digit * nw, 0, (digit + 1) * nw, nh);
            tx -= dw;
            dst.set(tx, y, tx + dw, y + dh);
            canvas.drawBitmap(bitmap, src, dst, null);
            value /= 10;
        } while (value > 0);
    }
    public void draw(Canvas canvas, float multi) {
        int value = this.displaynum;
        int nw = bitmap.getWidth() / 10;
        int nh = bitmap.getHeight();
        int dw = (int) (nw * GameView.MULTIPLIER * multi);
        int dh = (int) (nh * GameView.MULTIPLIER * multi);
        int tx = x;
        do {
            int digit = value % 10;
            src.set(digit * nw, 0, (digit + 1) * nw, nh);
            tx -= dw;
            dst.set(tx, y, tx + dw, y + dh);
            canvas.drawBitmap(bitmap, src, dst, null);
            value /= 10;
        } while (value > 0);
    }


}
