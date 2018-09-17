declare var $: any;

import loadCWasm from './out.js';

const CMD_NAME = 'abs';

const getCommandOutput = (args, jqconsole) => {
  if (args[0] === '') {
    return '';
  }

  const cmd = args[0];
  if (cmd === CMD_NAME) {
    // Call our driver function with the arguments and retrieve the output
    const output = 'TODO\n'; // TODO
    return output;
  } else if (cmd === 'clear') {
    jqconsole.Reset();
    return null;
  }

  return `command not found: ${cmd}\n`;
};

const setupConsole = () => {
  const jqconsole = ($('#console') as any).jqconsole('Virtual console loaded\n', '»');
  const startPrompt = () =>
    jqconsole.Prompt(true, (input: string) => {
      const args = input.split(' ');
      const output = getCommandOutput(args, jqconsole);

      if (output !== null) {
        jqconsole.Write(output, 'jqconsole-output');
      }

      // Restart the prompt.
      startPrompt();
    });
  startPrompt();
};

const init = async loaded => {
  const foo = () => loaded._foo.call(loaded);
  foo();

  setupConsole();
};

loadCWasm().then(init);
