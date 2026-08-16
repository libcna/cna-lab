package com.openeggbert.cna.template;

public class Main {
    public static void main(String[] args) {
        try (HelloGame game = new HelloGame()) {
            game.run();
        }
    }
}
