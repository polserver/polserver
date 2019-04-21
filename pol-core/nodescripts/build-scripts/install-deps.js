
// @ts-check

const fs = require('fs'), util = require('util'), https = require('https'), 
    path = require('path'), child_process = require('child_process'), os = require('os');

async function run() {

    if (!fs.existsSync("node_modules")) fs.mkdirSync("node_modules");

    const package = JSON.parse(await util.promisify(fs.readFile)("./package-lock.json", "utf-8"));
    for (const [packageName, packageDesc] of Object.entries(package.dependencies)) {

        // Try to load package
        try {
            require(packageName);
            continue;
        } catch (e) {
            // Fallthrough to download
        }

        // Set up download
        const downloadDestPath = os.tmpdir(),
            outputDestPath = path.resolve(__dirname, "./node_modules"),
            fileBase = path.basename(packageDesc.resolved),
            downloadDestFileName = path.join(downloadDestPath, fileBase),
            file = fs.createWriteStream(downloadDestFileName);

        // Download
        await new Promise((resolve, reject) => {
            const request = https.get(packageDesc.resolved, function (response) {
                response.pipe(file);
                console.log("Done!");
            });
            file.on("finish", () => { resolve() });
            file.on("error", () => { reject() });
        });

        // Unzip
        await new Promise((resolve, reject) => {
            const proc = child_process.spawn("tar", ["xvfz", downloadDestFileName, "-C", downloadDestPath]);
            proc.on('close', (code) => { if (code) reject(); else resolve(); });
        });

        // Get module name
        const unzippedDir = path.join(downloadDestPath, "package");
        const modulePackage = JSON.parse(await util.promisify(fs.readFile)(path.join(unzippedDir, "/package.json"), "utf-8"));

        // Move 
        fs.renameSync(path.join(downloadDestPath, "package"), path.join(outputDestPath, modulePackage.name));
    }
}

module.exports.install = run;

if (require.main == module)
    run().catch(e => { console.error("Error", e); process.exit(1); });