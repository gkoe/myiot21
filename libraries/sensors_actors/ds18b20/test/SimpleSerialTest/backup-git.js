"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const child_process_1 = require("child_process");
child_process_1.exec('git init', (error, response) => {
    console.log(error, response);
    child_process_1.exec('git add -A', (error, response) => {
        child_process_1.exec(`git commit -m "${new Date().toISOString()}"`, (error, response) => {
            if (error) {
                console.log(`!!! ERROR: ${error}`);
            }
            else {
                child_process_1.exec('git archive -o latest.zip HEAD', (error, response) => {
                    if (error) {
                        console.log(`!!! ERROR: ${error}`);
                    }
                    else {
                        console.log('Project-Zip created');
                    }
                });
            }
        });
    });
});
