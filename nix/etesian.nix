{ generic, ... }:
let pkg =
  { doxygen, qt4, coriolis-coloquinte, coriolis-vlsisapd, coriolis-hurricane, coriolis-crlcore }:
  {
    name = "etesian";
    src = ../etesian;
    buildInputs = [
      qt4 coriolis-coloquinte coriolis-vlsisapd
      coriolis-hurricane coriolis-crlcore
    ];
    nativeBuildInputs = [ doxygen ];
    pythonImportsCheck = [ "Etesian" ];
  };
in generic pkg
