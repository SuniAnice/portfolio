package kr.ac.kpu.game.s2016182019.termproject.utils;

import kr.ac.kpu.game.s2016182019.termproject.game.object.Block;

public class SwapHelper {
    public static void swap(Block o1, Block o2) {
        o1.moveto(o2.x, o2.y);
        o2.moveto(o1.x, o1.y);
    }
}
