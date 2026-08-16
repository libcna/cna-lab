package com.openeggbert.cna.template.android;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import com.openeggbert.cna.template.HelloGame;

/**
 * Android Activity for HelloGame.
 * In a real CNA-Java project, this might extend a specialized Activity class.
 */
public class HelloGameActivity extends AppCompatActivity {
    private HelloGame game;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // In CNA/XNA for Java, the game would typically be initialised here
        // and attached to a view or a native surface.
        game = new HelloGame();
        // game.Run(); 
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (game != null) {
            game.close();
        }
    }
}
