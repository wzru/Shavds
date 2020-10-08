import React, { useEffect } from "react";
import css from "./code.module.scss";
import hljs from "highlight.js";
import "highlight.js/styles/default.css";

export default function Code(props) {
  const { code, name } = props;
  useEffect(() => {
    document.querySelectorAll("pre code").forEach((block) => {
      hljs.highlightBlock(block);
    });
  });
  return (
    <div className={css["index"]}>
      <pre className={css["pre"]}>
        {code ? code.split('\n').map((line, index) => {
          return (
            <div key={index + line} className={css['pre-line']}>
              <div className={`${css['pre-line-bg']} ${name}-${index + 1}`}></div>
              <code>{index + 1}</code>
              <code className={name.split(".")[1]}>{line}</code>
            </div>
          )
        }) : <code>'Hello World'</code>}
      </pre>
    </div>
  );
}
