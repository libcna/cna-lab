package com.openeggbert.cna.template.teavm;

import com.openeggbert.cna.template.HelloGame;
import org.teavm.jso.browser.Window;

public class TeaVMLauncher {
    public static void main(String[] args) {
        Window.current().alert("TeaVM Launcher: Initializing HelloGame...");
        
        try (HelloGame game = new HelloGame()) {
            game.Run();
        } catch (Exception e) {
            Window.current().alert("Error running game: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
