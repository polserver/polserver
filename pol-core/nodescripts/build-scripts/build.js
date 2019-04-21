// @ts-check

const generator = require("./wrap-generator");

async function run() {
  await generator.generateModules();
  await generator.generateObjects();
}

if (require.main === module) {
  run().catch(e => { 
    console.error("Error",e); 
    process.exit(1);
  });
}
