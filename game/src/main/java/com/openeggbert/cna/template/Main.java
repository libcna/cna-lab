package com.openeggbert.cna.template;

public class Main {
    public static void main(String[] args) {
        boolean smokeTest = false;
        for (String arg : args) {
            if ("--smoke-test".equals(arg)) {
                smokeTest = true;
                break;
            }
        }

        try (HelloGame game = new HelloGame(smokeTest)) {
            game.Run();
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
