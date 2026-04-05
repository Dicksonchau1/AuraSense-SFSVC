import { createBrowserRouter, RouterProvider, ScrollRestoration } from 'react-router-dom';
import { SiteShell } from '../components/layout/SiteShell';
import { HomePage } from '../pages/HomePage';
import { PlatformPage } from '../pages/PlatformPage';
import { ProductsPage } from '../pages/ProductsPage';
import { ProductSFSVCPage } from '../pages/ProductSFSVCPage';
import { ProductNERMNPage } from '../pages/ProductNERMNPage';
import { ProductNSSIMPage } from '../pages/ProductNSSIMPage';
import { FAQPage } from '../pages/FAQPage';
import { LegalPage } from '../pages/LegalPage';
import { ContactPage } from '../pages/ContactPage';

const router = createBrowserRouter([
  {
    element: (
      <>
        <SiteShell />
        <ScrollRestoration />
      </>
    ),
    children: [
      { path: '/', element: <HomePage /> },
      { path: '/platform', element: <PlatformPage /> },
      { path: '/products', element: <ProductsPage /> },
      { path: '/products/sfsvc', element: <ProductSFSVCPage /> },
      { path: '/products/nermn', element: <ProductNERMNPage /> },
      { path: '/products/nssim', element: <ProductNSSIMPage /> },
      { path: '/faq', element: <FAQPage /> },
      { path: '/legal/:slug', element: <LegalPage /> },
      { path: '/contact', element: <ContactPage /> },
    ],
  },
]);

export function AppRouter() {
  return <RouterProvider router={router} />;
}
