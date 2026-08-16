import { HelloGame } from "./HelloGame";
import { bindingsAvailable } from "@openeggbert/cna-js";

async function main() {
    console.log("cna-js-template: starting...");
    
    if (!bindingsAvailable) {
        console.warn("CNA bindings not available in this environment.");
    }

    const game = new HelloGame();
    
    // Check for smoke-test in command line arguments (Node.js)
    if (typeof process !== 'undefined' && process.argv.includes('--smoke-test')) {
        game.SetSmokeTest(true);
    }

    try {
        await game.Run();
    } catch (err) {
        console.error("Game crashed:", err);
        if (typeof process !== 'undefined') {
            process.exit(1);
        }
    }
}

main();
