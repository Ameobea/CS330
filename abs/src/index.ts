declare var $: any;

import loadCWasm from './out.js';

const CMD_NAME = 'abs';

const getCommandOutput = (args, jqconsole, abs) => {
  if (args[0] === '') {
    return '';
  }

  const cmd = args[0];
  if (cmd === CMD_NAME) {
    // Call our driver function with the arguments and retrieve the output
    const output = abs(args).join('\n');
    return output + '\n';
  } else if (cmd === 'clear') {
    jqconsole.Reset();
    return null;
  }

  return `command not found: ${cmd}\n`;
};

const init = loaded => {
  (window as any).MODULE = loaded;
  const abs = (args: string[]): string[] => {
    const absInner = (argc, argv) => loaded._driver.call(loaded, argc, argv);

    const stringPointers = new Uint32Array(
      [...args, null].map(arg => {
        if (arg === null) {
          return 0;
        }

        const ptr = loaded._malloc(arg.length * 4 + 1);
        loaded.stringToUTF8(arg, ptr, arg.length * 4 + 1);
        return ptr;
      })
    );
    const ptr = loaded._malloc(stringPointers.length * 4);
    loaded.HEAPU32.set(stringPointers, ptr >> 2);
    const returnValue = absInner(args.length, ptr); // **char
    stringPointers.forEach(loaded._free);

    let curHeapIndex = returnValue;
    const returnedStrings: string[] = [];
    while (loaded.HEAP32[curHeapIndex >> 2] !== 0) {
      const convertedString = loaded.Pointer_stringify(loaded.HEAP32[curHeapIndex >> 2]);
      returnedStrings.push(convertedString);
      // console.log(`Freeing ${loaded.HEAP32[curHeapIndex >> 2]}`);
      loaded._free(curHeapIndex);
      curHeapIndex += 4;
    }
    loaded._free(ptr);

    return returnedStrings;
  };

  const jqconsole = ($('#console') as any).jqconsole('Virtual console loaded\n', '»');
  const startPrompt = () =>
    jqconsole.Prompt(true, (input: string) => {
      const args = input.split(' ');
      const output = getCommandOutput(args, jqconsole, abs);

      if (output !== null) {
        jqconsole.Write(output, 'jqconsole-output');
      }

      // Restart the prompt.
      startPrompt();
    });
  startPrompt();
};

loadCWasm().then(init);
