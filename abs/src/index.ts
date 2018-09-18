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

type pointer = number;

interface Module {
  HEAP8: Int8Array;
  HEAPU8: Uint8Array;
  HEAPU32: Uint32Array;
  HEAP32: Int32Array;
  ENV: { [varName: string]: string };
  _malloc: (bytes: number) => pointer;
  _free: (ptr: pointer) => void;
  _driver: (argc: number, argv: pointer) => pointer;
  // preamble functions
  stringToUTF8: (s: string, ptr: pointer, length: number) => void;
  Pointer_stringify: (ptr: pointer) => string;
}

const init = (loaded: Module) => {
  // loaded.ENV['SHELL'] = 'jqconsole';
  (window as any).MODULE = loaded;
  const abs = (args: string[]): string[] => {
    const absInner = (argc: number, argv: pointer) => loaded._driver.call(loaded, argc, argv);

    const stringPointers = new Int32Array(
      [...args, null].map(arg => {
        if (arg === null) {
          return 0;
        }

        const ptr = loaded._malloc(arg.length * 4 + 1);
        loaded.stringToUTF8(arg, ptr, arg.length * 4 + 1);
        return ptr;
      })
    );

    // 32-bit pointers; 4 bytes each
    const argvPtr = loaded._malloc(stringPointers.length * stringPointers.BYTES_PER_ELEMENT);
    loaded.HEAP32.set(stringPointers, argvPtr >> 2);
    const returnValue: pointer = absInner(args.length, argvPtr); // **char
    stringPointers.forEach(loaded._free);

    let curHeapIndex = returnValue;
    const returnedStrings: string[] = [];
    while (loaded.HEAP32[curHeapIndex >> 2] !== 0) {
      const convertedString = loaded.Pointer_stringify(loaded.HEAP32[curHeapIndex >> 2]);
      returnedStrings.push(convertedString);
      loaded._free(loaded.HEAP32[curHeapIndex >> 2]);
      curHeapIndex += 4;
    }
    loaded._free(argvPtr);

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
