import React, {useEffect} from 'react';
import {useBaseUrlUtils} from '@docusaurus/useBaseUrl';

// Redirect /api/ route to the generated Doxygen API index served from static assets
export default function ApiRedirect(): JSX.Element {
  const {withBaseUrl} = useBaseUrlUtils();
  const apiIndex = withBaseUrl('/api/index.html');

  useEffect((): void => {
    window.location.replace(apiIndex);
  }, [apiIndex]);

  return (
    <main className="container margin-vert--lg">
      <h1>Redirecting to API Reference…</h1>
      <p>
        If you are not redirected automatically,
        {' '}
        <a href={apiIndex}>click here</a>.
      </p>
    </main>
  );
}
