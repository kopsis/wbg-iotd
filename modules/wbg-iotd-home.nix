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

  systemd.user.services = lib.mkIf cfg.services.wbg-iotd.enable {
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
