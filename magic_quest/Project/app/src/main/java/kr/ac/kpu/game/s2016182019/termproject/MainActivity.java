package kr.ac.kpu.game.s2016182019.termproject;

import android.os.Bundle;
import android.util.DisplayMetrics;

import androidx.appcompat.app.AppCompatActivity;

import kr.ac.kpu.game.s2016182019.termproject.framework.game.MainGame;
import kr.ac.kpu.game.s2016182019.termproject.framework.view.GameView;

public class MainActivity extends AppCompatActivity {

    private MainGame basegame;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        basegame = new MainGame();


        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onPause() {
        super.onPause();
        GameView.view.pauseGame();
    }

    @Override
    protected void onResume() {
        GameView.view.resumeGame();
        super.onResume();
    }

    @Override
    public void onBackPressed() {
        if (GameView.view.handleBackKey()) {
            return;
        }
        super.onBackPressed();
    }
}