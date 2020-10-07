import React from "react";
import css from "./progress.module.scss";

export default function Progress(props) {
  const { progress, num } = props;
  return (
    <div className={css["index"]}>
      <div className={css["title"]}>
        总进度 {Object.keys(progress).filter((filenames) => progress[filenames] === 1).length}/{num * (num - 1) * 0.5}
      </div>
      <div className={css["progress"]}>
        {Object.keys(progress).map((filenames) => {
          return (
            <div className={css["progress-item"]} key={filenames}>
              <div className={css["progress-item-name"]}>{filenames}</div>
              <div className={css["progress-item-bar"]}>
                <div
                  className={css["progress-item-bar-content"]}
                  style={{ width: `${progress[filenames] * 100}%` }}
                ></div>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
}
