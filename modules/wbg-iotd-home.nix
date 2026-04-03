{
  config,
  options,
  lib,
  pkgs,
  ...
}:
with lib;
let
  cfg = config.services.wbg-iotd;
  wbg-iotd-bin = lib.getExe config.wbg-iotd.package;
in
  options.services.wbg-iotd = {
    enable = mkOption {
      type = types.bool;
      default = false;
      description = ''
        Enable the wallpaper image-of-the-day service.
      '';
    };
  };

  config = lib.mkIf cfg.services.wbg-iotd.enable {
    systemd.user.services = {
      wbg-iotd = {
        Unit = {
          Description = "Wallpaper image-of-the-day manager.";
          PartOf = [ "graphical-session.target" ];
          After = [ "graphical-session.target" ];
        };
        Service = {
          Type = "exec";
          ExecStart = "${wbg-iotd-bin}";
          Restart = "always";
          RestartSec = 10;
        };
        Install = {
          WantedBy = [ "graphical-session.target" ];
        };
      };
    };
  };
};
