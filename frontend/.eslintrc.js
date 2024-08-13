module.exports = {
  "env": {
    "browser": true,
    "es2021": true
  },
  "extends": [
		"preact",
    "eslint:recommended",
    'plugin:@typescript-eslint/recommended',
    'plugin:react/recommended',
    "prettier",
  ],
  "overrides": [
    {
      "env": {
        "node": true
      },
      "files": [
        ".eslintrc.{js,cjs}"
      ],
      "parserOptions": {
        "sourceType": "script",
      }
    }
  ],
  parser: '@typescript-eslint/parser',
  "parserOptions": {
    "project": "tsconfig.json",
    "ecmaVersion": "latest",
    "sourceType": "module",
  },
  "plugins": [
    '@typescript-eslint',
    "preact"
  ],
  "root": true,
  "rules": {
    "react/react-in-jsx-scope": "off",
    "react/jsx-uses-react": "error",
  },
  "settings": {
    "react": {
      "version": "18.2.0"
    }
  }
}
