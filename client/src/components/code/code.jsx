import React, { useState, useEffect } from "react";
import css from "./code.module.scss";
import hljs from 'highlight.js';
import 'highlight.js/styles/default.css';

export default function Code(props) {
  const { code, name } = props;
  useEffect(() => {
    document.querySelectorAll("pre code").forEach(block => {
      hljs.highlightBlock(block);
    });
  })
  return (
    <div className={css["index"]}>
      <pre className={css['pre']}>
        <code className={name.split('.')[1]}>
          {code ? code : "Hello World!"}
        </code>
      </pre>
    </div>
  )
}
