package com.openeggbert.cna.template;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.user.client.Window;

public class GWTLauncher implements EntryPoint {
    @Override
    public void onModuleLoad() {
        Window.alert("GWT Launcher: Initializing HelloGame...");
        
        try (HelloGame game = new HelloGame()) {
            game.Run();
        } catch (Exception e) {
            Window.alert("Error running game: " + e.getMessage());
        }
    }
}
