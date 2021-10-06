importScripts("gnugo.js");

let gnugoInstance;

function postGtp(cmdStr) {
  gnugoInstance.ccall("post_gtp", "int", ["string"], [cmdStr]);
}

self.onmessage = ({data}) => {
  if (typeof data == "string")
    postGtp(data);
  else {
    switch (data.cmd) {
      case "init":
        GnuGo({
          preRun: ({FS}) => {
            let outBuf = "";
            function stdout(char) {
              switch (char) {
                case 0:
                case 0x0a: self.postMessage(outBuf); outBuf = ""; return;
                case 0x0d: return;
                default:   outBuf += String.fromCharCode(char);
              }
            }
            FS.init(null, stdout, stdout);
          },
          arguments: [ "--mode", "gtp" ]
        }).then(gg => {
          gnugoInstance = gg;
          self.postMessage({type: "status", status: "ready"});
        });
        break;

      case "loadsgf":
        fetch(data.url || data.file).then(res => res.text())
        .then(sgfText => {
          gnugoInstance.FS.writeFile(data.file, sgfText);
          postGtp("loadsgf " + data.file);
          if (data.then)
            postGtp(data.then);
        });
        break;
    }
  }
}
